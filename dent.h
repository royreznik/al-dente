#ifndef DENT_H
#define DENT_H

#include <pthread.h>

// Maximum path length
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Work queue node structure
typedef struct work_node {
    char *path;
    struct work_node *next;
} work_node_t;

// Work queue structure
typedef struct {
    work_node_t *head;
    work_node_t *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int tasks_in_progress; // Number of tasks being processed or pending
} work_queue_t;

// Function declarations
void queue_init(work_queue_t *queue);
void queue_push(work_queue_t *queue, char *path);
char *queue_pop(work_queue_t *queue);
void task_done(work_queue_t *queue);
void queue_destroy(work_queue_t *queue);

void list_directory(const char *path, work_queue_t *queue);
void *worker_function(void *arg);

#endif // DENT_H