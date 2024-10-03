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
    int fd = open(path, O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        perror("open");
        return;
    }

    char buf[BUF_SIZE];
    ssize_t nread;

    while ((nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE)) > 0) {
        int bpos = 0;

        while (bpos < nread) {
            struct linux_dirent64 *d = (struct linux_dirent64 *)(buf + bpos);
            bpos += d->d_reclen;

            // Skip "." and ".."
            if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
                continue;

            char new_path[PATH_MAX];
            snprintf(new_path, sizeof(new_path), "%s/%s", path, d->d_name);
            printf("%s\n", new_path);

            if (d->d_type == DT_DIR) {
                // It's a directory
                char *new_path_copy = strdup(new_path);
                if (!new_path_copy) {
                    perror("strdup");
                    exit(EXIT_FAILURE);
                }
                queue_push(queue, new_path_copy);
            }
            // Entries with unknown type are ignored to avoid stat
        }
    }

    if (nread == -1) {
        perror("getdents64");
    }

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
