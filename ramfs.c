#include "ramfs.h"
#include <stdbool.h>

/* modify this file freely */

typedef struct File{
    /* Common properties */
    bool isDirectory;
    char *fileName;
    /* Directory properties */
    bool haveChild;
    struct File *firstChildFile;
    /* File properties */
    int offset;
    struct File *nextFile;
}file;

int ropen(const char *pathname, int flags) {
    // TODO();
}

int rclose(int fd) {
    // TODO();
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    // TODO();
}

ssize_t rread(int fd, void *buf, size_t count) {
    // TODO();
}

off_t rseek(int fd, off_t offset, int whence) {
    // TODO();
}

int rmkdir(const char *pathname) {
    // TODO();
}

int rrmdir(const char *pathname) {
    // TODO();
}

int runlink(const char *pathname) {
    // TODO();
}

void init_ramfs() {
    // TODO();
}
