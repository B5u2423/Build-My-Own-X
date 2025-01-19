#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <openssl/sha.h>
#include <sys/stat.h>

#ifndef BLOB_H
#define BLOB_H

#define CHUNK   16384
#define OBJ_DIR ".git/objects"
#define SHA_DIGEST_STRING_LEN SHA_DIGEST_LENGTH * 2

void get_file_path (char **full_path, const char* hash);
int cat_file (FILE *source);
int hash_object(FILE *source);
size_t get_content_len (FILE *fp);

#endif