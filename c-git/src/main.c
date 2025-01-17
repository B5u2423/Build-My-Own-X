#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "blob.h"

int main(int argc, char *argv[]) {
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 2) {
        fprintf(stderr, "Usage: ./my-git <command> [<args>]\n");
        return 1;
    }
    
    const char *command = argv[1];
    
    if (strcmp(command, "init") == 0) {
        if (mkdir(".git", 0755) == -1 || 
            mkdir(OBJ_DIR, 0755) == -1 || 
            mkdir(".git/refs", 0755) == -1) {
            fprintf(stderr, "Failed to create directories: %s\n", strerror(errno));
            return 1;
        }
        
        FILE *headFile = fopen(".git/HEAD", "w");
        if (headFile == NULL) {
            fprintf(stderr, "Failed to create .git/HEAD file: %s\n", strerror(errno));
            return 1;
        }
        fprintf(headFile, "ref: refs/heads/main\n");
        fclose(headFile);
        
        printf("Initialized git directory\n");
    } else if (strcmp(command, "cat-file") == 0) {
        if (strcmp(argv[2], "-p") != 0 || argv[3] == NULL) {
            fprintf(stderr, "Usage: ./my-git cat-file -p <object>\n");
            exit(1);
        }
        char *full_path = malloc(sizeof(char) * (SHA_LEN + 2 + strlen(OBJ_DIR)));
        FILE *blob_file = NULL;
        
        get_file_path(&full_path, argv[3]);
        blob_file = fopen(full_path, "rb");
        if (blob_file == NULL) {
            perror("Cannot open file");
            exit(1);
        }
        // printf("%s", full_path); // log
        cat_file(blob_file);

        free(full_path);
        fclose(blob_file);
    } else if (strcmp(command, "hash-object") == 0) {
        if (strcmp(argv[2], "-w") != 0 || argv[3] == NULL) {
            fprintf(stderr, "Usage: ./my-git hash-object -w <file>\n");
            exit(1);
        }
        char *full_path= NULL;
        FILE *blob_dest = NULL;
        blob_dest = fopen(full_path, "wb");
        if (blob_dest == NULL) {
            perror("Cannot open file");
            exit(1);
        }

        fclose(blob_dest);
    } else {
        fprintf(stderr, "Unknown command %s\n", command);
        return 1;
    }
    
    return 0;
}
