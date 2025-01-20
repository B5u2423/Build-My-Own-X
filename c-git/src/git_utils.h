#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <openssl/sha.h>
#include <sys/stat.h>

#ifndef GIT_UTILS_H
#define GIT_UTILS_H

#define CHUNK   16384
#define OBJ_DIR ".git/objects"
#define SHA_DIGEST_STRING_LEN SHA_DIGEST_LENGTH * 2

char *get_file_path (const char* hash);
size_t get_content_len (FILE *fp);
int cat_file (FILE *source);
int hash_object(FILE *source);

#endif