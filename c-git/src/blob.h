#ifndef BLOB_H
#define BLOB_H

#define CHUNK   16384
#define SHA_LEN 40
#define OBJ_DIR ".git/objects"

void get_file_path (char **full_path, const char* hash);
int cat_file (FILE *source);
void hash_object(FILE *source);

#endif