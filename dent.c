#include "dent.h"

sem_t semaphore;

void *listDir(void *arg) {
    char *path = (char *)arg;
    listDirThreaded(path);
    free(path);
    return NULL;
}

void listDirThreaded(const char *path) {
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent64 *d;
    int bpos;

    fd = open(path, O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        perror("open");
        sem_post(&semaphore);
        pthread_exit(NULL);
    }

    for (;;) {
        nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
        if (nread == -1) {
            perror("getdents64");
            close(fd);
            sem_post(&semaphore);
            pthread_exit(NULL);
        }

        if (nread == 0) {
            break;
        }

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent64 *) (buf + bpos);
            if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0) {
                printf("%s/%s\n", path, d->d_name);

                if (d->d_type == DT_DIR || d->d_type == DT_UNKNOWN) {
                    char new_path[1024];
                    snprintf(new_path, sizeof(new_path), "%s/%s", path, d->d_name);

                    sem_wait(&semaphore); // Wait for an available slot
                    pthread_t tid;
                    char *new_path_copy = strdup(new_path);
                    pthread_create(&tid, NULL, listDir, new_path_copy);
                    pthread_detach(tid);
                }
            }
            bpos += d->d_reclen;
        }
    }

    close(fd);
    sem_post(&semaphore); // Release the semaphore
    pthread_exit(NULL);
}