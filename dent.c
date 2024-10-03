#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "dent.h"  // Assuming this header defines struct linux_dirent64
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>

// Buffer size for getdents64
#define BUF_SIZE 4096


struct linux_dirent64 {
  ino64_t d_ino;
  off64_t d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[];
};

// Initialize the work queue
void queue_init(work_queue_t *queue) {
    queue->head = NULL;
    queue->tail = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->tasks_in_progress = 0;
}

// Push a new path into the queue
void queue_push(work_queue_t *queue, char *path) {
    work_node_t *node = malloc(sizeof(work_node_t));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    node->path = path;
    node->next = NULL;

    pthread_mutex_lock(&queue->mutex);

    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;

    queue->tasks_in_progress++;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

// Pop a path from the queue
char *queue_pop(work_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);

    while (!queue->head && queue->tasks_in_progress > 0) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (!queue->head && queue->tasks_in_progress == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL; // No more work to do
    }

    work_node_t *node = queue->head;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }

    pthread_mutex_unlock(&queue->mutex);

    char *path = node->path;
    free(node);
    return path;
}

// Decrement the tasks_in_progress counter
void task_done(work_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->tasks_in_progress--;
    if (queue->tasks_in_progress == 0) {
        // No more tasks; wake up all threads
        pthread_cond_broadcast(&queue->cond);
    }
    pthread_mutex_unlock(&queue->mutex);
}

// Destroy the work queue
void queue_destroy(work_queue_t *queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}

// Function to list the contents of a directory using getdents64
void list_directory(const char *path, work_queue_t *queue) {
    int fd = open(path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    if (fd == -1) {
        perror("open");
        return;
    }

    // Allocate a large buffer on the heap to avoid stack overflow
    char *buf = malloc(BUF_SIZE);
    if (!buf) {
        perror("malloc");
        close(fd);
        return;
    }

    ssize_t nread;

    while ((nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE)) > 0) {
        // Use pointer arithmetic instead of indexing for efficiency
        char *ptr = buf;
        char *end = buf + nread;

        while (ptr < end) {
            struct linux_dirent64 *d = (struct linux_dirent64 *)ptr;
            ptr += d->d_reclen;

            // Skip "." and ".." entries
            if (d->d_name[0] == '.') {
                if (d->d_name[1] == '\0' ||
                    (d->d_name[1] == '.' && d->d_name[2] == '\0')) {
                    continue;
                }
            }

            // Precompute lengths to avoid multiple strlen calls
            size_t path_len = strlen(path);
            size_t name_len = d->d_reclen - offsetof(struct linux_dirent64, d_name) - 1;
            // Ensure name is null-terminated
            d->d_name[name_len] = '\0';

            // Allocate memory for new_path with minimal overhead
            char *new_path = malloc(path_len + 1 + name_len + 1);  // path + '/' + name + '\0'
            if (!new_path) {
                perror("malloc");
                continue;
            }

            // Construct the new path
            memcpy(new_path, path, path_len);
            new_path[path_len] = '/';
            memcpy(new_path + path_len + 1, d->d_name, name_len + 1);  // Include '\0'

            // Output the path
            printf("%s\n", new_path);

            // If it's a directory, push it onto the queue
            if (d->d_type == DT_DIR) {
                queue_push(queue, new_path);  // Transfer ownership of new_path
            } else {
                // Free memory if not adding to the queue
                free(new_path);
            }
            // Entries with unknown type are ignored to avoid stat
        }
    }

    if (nread == -1) {
        perror("getdents64");
    }

    free(buf);
    close(fd);
}

// Worker thread function
void *worker_function(void *arg) {
    work_queue_t *queue = (work_queue_t *)arg;
    while (1) {
        char *path = queue_pop(queue);
        if (path == NULL) {
            break; // No more work to do
        }

        list_directory(path, queue);
        free(path);

        task_done(queue);
    }
    return NULL;
}
