#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include "zlib.h"
#include "blob.h"

void get_file_path (char **full_path, const char* hash) {
    // Get the file path from the hash
    char file[SHA_LEN + 1];
    file[SHA_LEN] = '\0';
    
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

void hash_object(FILE *source) {
    // Get content size to allocate heap
    fseek(source, 0, SEEK_END);
    unsigned long src_size = ftell(source);
    fseek(source, 0, SEEK_SET);

    // printf("%ld\n", source_size);

    unsigned long long bufsize = CHUNK;
    char *blob_content = malloc(sizeof(char) * bufsize);
    char *src_content = malloc(sizeof(char) * src_size);
    
    // Append header
    sprintf(blob_content, "blob %ld", src_size);
    size_t header_len = strlen(blob_content); // Account for null byte
    blob_content[header_len++] = '\0';
    while (src_size > (bufsize - header_len)) { // Realloc if not enough
        bufsize *= 2;
        blob_content = realloc(blob_content, bufsize);
    }

    // Read source file content
    fread((void *)src_content, 1, (size_t) src_size, source);

    // Append to blob content
    strncat(blob_content + header_len, src_content, src_size + 1); // Null byte

    // printf("%s\n", source_content);
    // for (int i = 0; i < 100; i++) {
    //     if (dest_blob_content[i] == '\0') {
    //         printf("\\0");
    //         continue;
    //     }
    //     printf("%c", dest_blob_content[i]);
    // }

    // Zlib compress

    // Hash and get the dest file path
    // char *full_path= NULL;
    // FILE *blob_dest = fopen(full_path, "wb");
    // if (blob_dest == NULL) {
    //     fprintf(stderr, "Cannot open file %s: %s\n", full_path, strerror(errno));
    //     exit(1);
    // }
        
    // fclose(blob_dest);
    free(blob_content);
    free(src_content);
}