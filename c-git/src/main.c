#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "git_utils.h"

int main(int argc, char *argv[]) {
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 2) {
        fprintf(stderr, "Usage: ./my-git <command> [<args>]\n");
        exit(EXIT_FAILURE);
    }
    
    const char *command = argv[1];
    
    if (strcmp(command, "init") == 0) {
        if (mkdir(".git", 0755) == -1 || 
            mkdir(OBJ_DIR, 0755) == -1 || 
            mkdir(".git/refs", 0755) == -1) {
            fprintf(stderr, "Failed to create directories: %s\n", strerror(errno));
            exit(EXIT_FAILURE);

        }
        
        FILE *headFile = fopen(".git/HEAD", "w");
        if (headFile == NULL) {
            fprintf(stderr, "Failed to create .git/HEAD file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        fprintf(headFile, "ref: refs/heads/main\n");
        fclose(headFile);
        
        printf("Initialized git directory\n");
    } else if (strcmp(command, "cat-file") == 0) {
        if (strcmp(argv[2], "-p") != 0 || argv[3] == NULL) {
            fprintf(stderr, "Usage: ./my-git cat-file -p <object>\n");
            exit(EXIT_FAILURE);
        }
        char *full_path = get_file_path(argv[3]);
        cat_file(full_path);
        free(full_path);
    } else if (strcmp(command, "hash-object") == 0) {
        if (strcmp(argv[2], "-w") != 0 || argv[3] == NULL) {
            fprintf(stderr, "Usage: ./my-git hash-object -w <file>\n");
            exit(EXIT_FAILURE);
        }
        char *hash_buf = hash_object(argv[3]);
        if (hash_buf == NULL) {
            fprintf(stderr, "error hashing file %s\n", argv[3]);
            exit(EXIT_FAILURE);
        }
        sha2hex(hash_buf);
        free(hash_buf);
    } else if (strcmp(command, "ls-tree") == 0) {
        if (argv[2]  == NULL) {
            fprintf(stderr, "Usage: ./my-git ls-tree <hash>\n");
            exit(EXIT_FAILURE);
        }
        char *full_path = get_file_path(argv[2]);
        ls_tree(full_path);
        free(full_path);
    } else if (strcmp(command, "write-tree") == 0) {
        write_tree();
    } else {
        fprintf(stderr, "Unknown command %s\n", command);
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
