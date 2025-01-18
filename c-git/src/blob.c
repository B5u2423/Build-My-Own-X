#include "zlib.h"
#include "blob.h"

void get_file_path (char **full_path, const char* hash) {
    // Get the file path from the hash
    char file[SHA_DIGEST_STRING_LEN + 1];
    file[SHA_DIGEST_STRING_LEN] = '\0';
    
    strncpy(file, hash, strlen(hash));
    sprintf(*full_path, "%s/%.2s/%s", OBJ_DIR, hash, file + 2);
}

int cat_file (FILE *source) {
    // Zlib decompressed the blob file
    int ret;
    z_stream stream;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;

    ret = inflateInit(&stream);
    if (ret != Z_OK) {
        fprintf(stderr, "Cannot decompressed file");
        return ret;
    }

    do {
        stream.avail_in = fread(in, 1, CHUNK, source);
        if(ferror(source)) {
            (void)inflateEnd(&stream);
            return Z_ERRNO;
        }
        if (stream.avail_in == 0) break;
        stream.next_in = in;

        do {
            stream.avail_out = CHUNK;
            stream.next_out = out;
            ret = inflate(&stream, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
                case Z_NEED_DICT:
                    return Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&stream);
                    return ret;
            }
        } while (stream.avail_out == 0);

    } while (ret != Z_STREAM_END);
    (void)inflateEnd(&stream);

    // blob {content_byte_size}\0{content}
    // Extract the content string    
    char *content = strchr((const char *)out, '\0') + 1;
    printf("%s", content);
    return 0;

}

int hash_object(FILE *source) {
    // Get content size to allocate heap
    fseek(source, 0, SEEK_END);
    size_t src_len = ftell(source);
    fseek(source, 0, SEEK_SET);

    unsigned long long bufsize = CHUNK;
    size_t blob_content_buf_size = sizeof(char) * bufsize; 
    size_t src_content_size = sizeof(char) * (src_len + 1);
    
    char *blob_content_buf = malloc(blob_content_buf_size);
    char *src_content = malloc(src_content_size); // null byte

    memset(blob_content_buf, 0, blob_content_buf_size);
    memset(src_content, 0, src_content_size);

    if (src_content == NULL || blob_content_buf == NULL) {
        fprintf(stderr, "Error allocating memory");
        return 1;
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
    unsigned char digest[SHA_DIGEST_LENGTH]; 
    unsigned char path_buf[SHA_DIGEST_STRING_LEN];
    size_t path_len = SHA_DIGEST_STRING_LEN + 3 + strlen(OBJ_DIR);
    unsigned char *full_path = malloc(path_len);

    size_t blob_len = header_len + src_len;
    SHA1((const unsigned char *)blob_content_buf, blob_len, digest);

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        snprintf((char *)path_buf + i * 2, sizeof(path_buf), "%02x", digest[i]);
    }
    printf("%s\n", path_buf);

    int dir_path_len = strlen(OBJ_DIR) + 4;
    char dir_path[dir_path_len];
    snprintf(dir_path, dir_path_len, "%s/%.2s", OBJ_DIR, path_buf);
    dir_path[dir_path_len] = '\0';

    snprintf((char *)full_path, path_len, "%s/%.2s/%s", OBJ_DIR, path_buf, path_buf + 2);
 
    mkdir(dir_path, 0755);

    FILE *blob_dest = fopen((const char *)full_path, "wb+");
    if (blob_dest == NULL) {
        fprintf(stderr, "Cannot open file %s: %s\n", full_path, strerror(errno));
        return 1;
    }
        
    //-------------- Zlib compress --------------
    uLong compressed_len = compressBound(blob_len);
    unsigned char compressed_data[compressed_len];

    int ret = compress(compressed_data, &compressed_len, (const unsigned char *)blob_content_buf, blob_len);
    if (ret != Z_OK) {
        fprintf(stderr, "Compress error. Code %d", ret);
        return 1;
    }

    fwrite(compressed_data, 1, compressed_len, blob_dest);    

    fclose(blob_dest);

    free(blob_content_buf);
    free(src_content);
    free(full_path);

    return 0;
}