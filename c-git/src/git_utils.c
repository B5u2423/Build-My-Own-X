#define _DEFAULT_SOURCE
#include "zlib.h"
#include "git_utils.h"
#include <dirent.h>

/***********************
 * Utility
 ***********************/

char *get_file_path (const char* hash) {
    // Get the file path from the hash
    char file[SHA_DIGEST_STRING_LEN + 1];
    file[SHA_DIGEST_STRING_LEN] = '\0';

    size_t full_path_size = sizeof(char) * (SHA_DIGEST_STRING_LEN + 2 + strlen(OBJ_DIR));
    char *full_path = malloc(full_path_size);
    if (full_path == NULL) {
        fprintf(stderr, "error allocating memory in %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    memset(full_path, 0, full_path_size);
    
    strncpy(file, hash, strlen(hash));
    sprintf(full_path, "%s/%.2s/%s", OBJ_DIR, hash, file + 2);

    return full_path;
}

size_t get_content_len (FILE *fp) {
    fseek(fp, 0, SEEK_END);
    size_t ret = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return ret;
}

unsigned char *uncompress_zlib_object (FILE *source) {
    size_t compressed_data_len = get_content_len(source) + 1;
    size_t out_buf_size = CHUNK;
   
    unsigned char in[compressed_data_len];
    unsigned char *out = malloc(out_buf_size);
    if (out == NULL) {
        fprintf(stderr, "error allocating memory in %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    fread(&in, 1, compressed_data_len, source);
    
    int ret = uncompress(out, (uLong *)&out_buf_size, (const unsigned char *)in, (uLong)compressed_data_len);
    // Keep increaseing the buffer size 
    while (ret == Z_BUF_ERROR) {
        out_buf_size *= 3;
        out = realloc(out, out_buf_size);
        ret = uncompress(out, (uLong *)&out_buf_size, (const unsigned char *)in, (uLong)compressed_data_len);
    }

    if (ret != Z_OK) {
        fprintf(stderr, "Uncompress error. Code %d\n", ret);
        return NULL;
    }

    return out;
}

/**
 * Idk why it worked but wow it works.
 */
void parse_n_print_tree_items (char *content_ptr, size_t content_len) {
    char mode_str[MODE_LEN];
    char name[1024];
    unsigned char hash[SHA_DIGEST_LENGTH + 1];

    char *substr_pointer = content_ptr;
    size_t bytes_read = 0;
    do {
        // Headers: blob|tree {content_size}\0
        size_t header_len = strlen(substr_pointer);

        // Parse the mode
        // mode_len also include the space between mode and content len.
        // eliminate the requirement of removing leading space for item name.
        size_t mode_len = (substr_pointer[0] == '4') ? 6 : 7; 
        strncpy(mode_str, substr_pointer, mode_len);
        long mode = atol(mode_str);

        if (mode == 40000) {
            printf("%06ld tree ", mode);
        } else if (mode == 100644 || mode == 100755) {
            printf("%06ld blob ", mode);
        }

        // Parse file/dir name
        substr_pointer += mode_len;
        bytes_read += mode_len;
        // name_len is arbitrary so it has to be calculated from the 
        // mode_len (fixed size) and the header_len
        size_t name_len = header_len - mode_len;
        strncpy(name, substr_pointer, name_len);
        name[name_len] = '\0';

        // Parse sha string
        substr_pointer += name_len + 1;
        bytes_read += name_len + 1;
        memcpy(hash, substr_pointer, SHA_DIGEST_LENGTH);
        hash[SHA_DIGEST_LENGTH] = '\0';
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            printf("%02x", hash[i]);
        }
        printf("\t%s\n", name);

        substr_pointer += SHA_DIGEST_LENGTH;
        bytes_read += SHA_DIGEST_LENGTH;
    } while (bytes_read < content_len);    
}

void sha2hex (unsigned char *digest) {
    for (size_t i = 0; i < SHA_DIGEST_LENGTH; i++) {
        printf("%02x", digest[i]);
    }
    putchar('\n');
}

/***********************
 * Blob object related
 ***********************/

void cat_file (char *file_path) {
    // Zlib decompressed the blob file
    FILE *blob_file = fopen(file_path, "rb");
    if (blob_file == NULL) {
        perror("Cannot open file");
        exit(1);
    }

    unsigned char *uncompressed_data = uncompress_zlib_object(blob_file);

    // blob {content_byte_size}\0{content}
    if (strncmp((char *)uncompressed_data, "blob", 4) != 0) {
        fprintf(stderr, "fatal: not a blob object\n");
        exit(1);
    }

    // Extract the content string    
    char *content = strchr((const char *)uncompressed_data, '\0') + 1;    
    printf("%s", content);

    free(uncompressed_data);
    fclose(blob_file);
}

unsigned char *hash_object(char *file_path) {
    FILE *source = fopen(file_path, "rb");
    if (source == NULL) {
        fprintf(stderr, "Cannot open file %s: %s\n", file_path, strerror(errno));
        return NULL;
    }

    // Get content size to allocate heap
    size_t src_len = get_content_len(source);

    unsigned long long bufsize = CHUNK;
    size_t blob_content_buf_size = bufsize; 
    size_t src_content_size = src_len + 1;
    
    char *blob_content_buf = malloc(blob_content_buf_size);
    if (blob_content_buf == NULL) {
        fprintf(stderr, "error allocating memory in %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    char *src_content = malloc(src_content_size); // null byte
    if (src_content == NULL) {
        fprintf(stderr, "error allocating memory in %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    unsigned char *digest = malloc(SHA_DIGEST_LENGTH);
    if (digest == NULL) {
        fprintf(stderr, "error allocating memory in %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    memset(blob_content_buf, 0, blob_content_buf_size);
    memset(src_content, 0, src_content_size);

    if (src_content == NULL || blob_content_buf == NULL) {
        fprintf(stderr, "Error allocating memory");
        exit(EXIT_FAILURE);
    }
    
    // Append header
    snprintf(blob_content_buf, sizeof(char) * bufsize, "blob %ld", src_len);
    size_t header_len = strlen(blob_content_buf);
    blob_content_buf[header_len++] = '\0';  // header has a null byte - update header length
    while (src_len > (bufsize - header_len)) { // Realloc if not enough
        bufsize *= 2;
        blob_content_buf = realloc(blob_content_buf, bufsize);
    }

    // Read source file content
    fread((void *)src_content, 1, (size_t) src_len, source);

    // Append to blob content
    strncat(blob_content_buf + header_len, src_content, src_len + 1); // null byte

    // Hash and get the dest file path
    size_t path_len = SHA_DIGEST_STRING_LEN + 3 + strlen(OBJ_DIR);
    unsigned char *full_path = malloc(path_len);
    if (full_path == NULL) {
        fprintf(stderr, "error allocating memory in %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    memset(full_path, 0, path_len);

    size_t blob_len = header_len + src_len;
    SHA1((const unsigned char *)blob_content_buf, blob_len, digest);

    int idx = snprintf((char *)full_path, path_len, "%s/%02x/", OBJ_DIR, digest[0]);
    full_path[idx] = '\0';
    mkdir((const char *)full_path, 0755);

    for (int i = 0; i < SHA_DIGEST_LENGTH - 1; i++) {
        snprintf((char *)(full_path + idx + i * 2), path_len, "%02x", digest[i + 1]);
    }
    full_path[SHA_DIGEST_LENGTH] = '\0';

    //-------------- Zlib compress --------------
    FILE *blob_dest = fopen((const char *)full_path, "wb+");
    if (blob_dest == NULL) {
        fprintf(stderr, "Cannot open file %s: %s\n", full_path, strerror(errno));
        return NULL;
    }
        
    uLong compressed_len = compressBound(blob_len);
    unsigned char compressed_data[compressed_len];

    int ret = compress(compressed_data, &compressed_len, (const unsigned char *)blob_content_buf, blob_len);
    if (ret != Z_OK) {
        fprintf(stderr, "Compress error. Code %d\n", ret);
        return NULL;
    }

    fwrite(compressed_data, 1, compressed_len, blob_dest);    

    fclose(blob_dest);
    fclose(source);

    free(blob_content_buf);
    free(src_content);
    free(full_path);

    return digest;
}

/***********************
 * Tree object related
 ***********************/
void ls_tree (char *file_path) {
    FILE *blob_file = fopen(file_path, "rb");
    if (blob_file == NULL) {
        perror("Cannot open file");
        exit(1);
    }

    unsigned char *uncompressed_data = uncompress_zlib_object(blob_file);

    // tree {content_byte_size}\0{tree_items}
    if (strncmp((char *)uncompressed_data, "tree", 4) != 0) {
        fprintf(stderr, "fatal: not a tree object\n");
        exit(1);
    }

    // find content len
    size_t content_len = atol(strchr((const char *)uncompressed_data, ' ') + 1); //null byte
    char *content_ptr = strchr((const char *)uncompressed_data, '\0') + 1;

    parse_n_print_tree_items(content_ptr, content_len);
}

void write_tree (void) {
    DIR* dir_ptr = opendir(".");
    if (dir_ptr == NULL) {
        perror("Cannot not open directory");
        exit(EXIT_FAILURE);
    }
    struct dirent **namelist;

    int n = scandir(".", &namelist, NULL, alphasort);
    if (n < 0) {
        perror("scandir()");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        if (namelist[i]->d_type == DT_DIR) {
            if (
                strcmp(namelist[i]->d_name, ".") == 0 ||
                strcmp(namelist[i]->d_name, "..") == 0 ||
                strcmp(namelist[i]->d_name, ".git") == 0
            ) {
                free(namelist[i]);
                continue;
            }
        }
        unsigned char *file_sha1 = hash_object(namelist[i]->d_name); 
        printf("%s\t\t%s\n", namelist[i]->d_name, "file_sha1");
        free(namelist[i]);
        free(file_sha1);
    }

    free(namelist);
    closedir(dir_ptr);
}