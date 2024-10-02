#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dent.h"

int main(int argc, char *argv[]) {
    char *start_dir = ".";
    int num_threads = 10; // Default number of threads

    // Parse command-line arguments
    if (argc > 1) {
        start_dir = argv[1];
    }
    if (argc > 2) {
        num_threads = atoi(argv[2]);
        if (num_threads <= 0) {
            fprintf(stderr, "Invalid value for max_concurrent_threads. It must be a positive integer.\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("%s", start_dir);

    // Initialize the work queue
    work_queue_t work_queue;
    queue_init(&work_queue);
    queue_push(&work_queue, strdup(start_dir));

    // Create worker threads
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; ++i) {
        if (pthread_create(&threads[i], NULL, worker_function, &work_queue) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all worker threads to finish
    for (int i = 0; i < num_threads; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    // Clean up
    free(threads);
    queue_destroy(&work_queue);
    return 0;
}
