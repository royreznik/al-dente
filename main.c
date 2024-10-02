#include "dent.h"

int main(int argc, char *argv[]) {
    const char *startDir = ".";
    int max_concurrent_threads = 1000;
    if (argc > 1) {
        startDir = argv[1];
    }

    if (argc > 2) {
        max_concurrent_threads = atoi(argv[2]);
        if (max_concurrent_threads <= 0) {
            fprintf(stderr, "Invalid value for max_concurrent_threads. It must be a positive integer.\n");
            exit(EXIT_FAILURE);
        }
    }
    sem_init(&semaphore, 0, max_concurrent_threads);
    printf("%s\n", startDir);
    listDirThreaded(startDir);
    pthread_exit(NULL);

    sem_destroy(&semaphore);

    return 0;
}