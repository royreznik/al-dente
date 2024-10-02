#ifndef DENT_H
#define DENT_H

#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define BUF_SIZE 16384

struct linux_dirent64 {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

extern sem_t semaphore;

void *listDir(void *arg);
void listDirThreaded(const char *path);

#endif // DENT_H