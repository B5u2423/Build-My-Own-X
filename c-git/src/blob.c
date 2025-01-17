#include <stdio.h>
#include <string.h>
#include "zlib.h"
#include <assert.h>
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
                    ret = Z_DATA_ERROR;
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

void hash_object(void) {

}