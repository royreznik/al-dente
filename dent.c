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

// Increase buffer sizes
#define READ_BUF_SIZE  (256 * 1024)  // 256 KB buffer for reading directory entries
#define WRITE_BUF_SIZE (256 * 1024)  // 256 KB buffer for output


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
    // Thread-local buffers to avoid malloc/free and improve cache locality
    static __thread char read_buf[READ_BUF_SIZE];
    static __thread char write_buf[WRITE_BUF_SIZE];
    static __thread size_t write_buf_pos = 0;
    static __thread char new_path[PATH_MAX];

    int fd = open(path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd == -1) {
        perror("open");
        return;
    }

    ssize_t nread;

    while ((nread = syscall(SYS_getdents64, fd, read_buf, READ_BUF_SIZE)) > 0) {
        char *ptr = read_buf;
        char *end = read_buf + nread;

        while (ptr < end) {
            struct linux_dirent64 *d = (struct linux_dirent64 *)ptr;
            ptr += d->d_reclen;

            // Prefetch next entry to improve cache performance
            __builtin_prefetch(ptr, 0, 1);

            // Skip "." and ".."
            if (d->d_name[0] == '.') {
                if (d->d_name[1] == '\0' ||
                    (d->d_name[1] == '.' && d->d_name[2] == '\0')) {
                    continue;
                }
            }

            // Calculate lengths without strlen
            size_t path_len = strlen(path);
            size_t name_len = d->d_reclen - offsetof(struct linux_dirent64, d_name) - 1;

            // Ensure name is null-terminated
            d->d_name[name_len] = '\0';

            // Check for path length overflow
            if (path_len + 1 + name_len >= PATH_MAX) {
                // Path too long; skip this entry
                continue;
            }

            // Construct new_path using thread-local buffer
            memcpy(new_path, path, path_len);
            new_path[path_len] = '/';
            memcpy(new_path + path_len + 1, d->d_name, name_len + 1);  // Copy null terminator

            // Accumulate output in write buffer
            size_t new_path_len = path_len + 1 + name_len;
            if (write_buf_pos + new_path_len + 1 > WRITE_BUF_SIZE) {
                // Flush the buffer
                fwrite(write_buf, 1, write_buf_pos, stdout);
                write_buf_pos = 0;
            }
            memcpy(write_buf + write_buf_pos, new_path, new_path_len);
            write_buf_pos += new_path_len;
            write_buf[write_buf_pos++] = '\n';

            if (d->d_type == DT_DIR) {
                // It's a directory; push it onto the queue
                char *path_copy = strdup(new_path);
                if (!path_copy) {
                    perror("strdup");
                    continue;
                }
                queue_push(queue, path_copy);
            }
            // Skip entries with unknown type to avoid stat
        }
    }

    if (nread == -1) {
        perror("getdents64");
    }

    // Flush any remaining output
    if (write_buf_pos > 0) {
        fwrite(write_buf, 1, write_buf_pos, stdout);
        write_buf_pos = 0;
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
