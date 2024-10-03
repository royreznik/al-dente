#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define DEPTH 4               // Depth of the directory tree
#define BREADTH 32            // Number of subdirectories and files at each level
#define MAX_PATH_LENGTH 4096

void create_structure(const char *base_path, int depth, int *counter);

int main(int argc, char *argv[]) {
    char *base_path = NULL;

    if (argc > 1) {
        base_path = argv[1];
    } else {
        fprintf(stderr, "Usage: %s <base_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create the base directory if it doesn't exist
    if (mkdir(base_path, 0777) == -1 && errno != EEXIST) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    int counter = 0; // Counter for total files and directories created

    create_structure(base_path, DEPTH, &counter);

    printf("Total files and directories created: %d\n", counter);

    return 0;
}

void create_structure(const char *base_path, int depth, int *counter) {
    if (depth == 0) {
        return;
    }

    char path[MAX_PATH_LENGTH];

    for (int i = 0; i < BREADTH; ++i) {
        // Create subdirectory
        snprintf(path, sizeof(path), "%s/dir_%d", base_path, i);
        if (mkdir(path, 0777) == -1 && errno != EEXIST) {
            perror("mkdir");
            continue;
        }
        (*counter)++;

        // Recursively create structure in the new directory
        create_structure(path, depth - 1, counter);

        // Create files in the current directory
        for (int j = 0; j < BREADTH; ++j) {
            snprintf(path, sizeof(path), "%s/file_%d.txt", base_path, j);
            FILE *fp = fopen(path, "w");
            if (fp) {
                fprintf(fp, "This is a test file.\n");
                fclose(fp);
                (*counter)++;
            } else {
                perror("fopen");
            }
        }
    }
}
