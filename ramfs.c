#include "ramfs.h"
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

/* To store all file and directories */
typedef struct File {
    /* Common properties */
    bool isDirectory;
    char *fileName;
    /* Directory properties */
    bool haveChild;
    struct File *firstChildFile;
    /* File properties */
    int offset;
    struct File *nextFile;
} file;

typedef struct Path {
    enum {
        FILE, DIRECTORY, ERROR
    } pathType;
    char **name;
    int nameCount;
} path;

bool checkNameValidity(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (!(isdigit(str[i]) || isalpha(str[i]) || str[i] == '.')) {
            return false;
        }
    }
    return true;
}

void destroyPath(path src) {
    for (int i = 0; i < src.nameCount; i++) {
        free(src.name[i]);
    }
    free(src.name);
}

path analyzePath(const char *pathname) {
    path ret;
    int len = (int) strlen(pathname);
    if (pathname[0] != '/') {
        ret.pathType = ERROR;
        return ret;
    }

    ret.name = malloc(sizeof(*ret.name) * 1024);
    ret.nameCount = 0;

    if (pathname[len - 1] == '/') {
        ret.pathType = DIRECTORY;
    } else {
        ret.pathType = FILE;
    }

    char *tmp = malloc(sizeof(char) * len);
    memset(tmp, 0, sizeof(*tmp));
    int tmpLen = 0;
    bool slashRead = 0;

    for (int i = 0; i <= len; i++) {
        if (pathname[i] == '/' && (!slashRead)) {
            slashRead = 1;
            if (checkNameValidity(tmp) && tmpLen) {
                ret.name[ret.nameCount++] = malloc(sizeof(char) * 1024);
                strcpy(ret.name[ret.nameCount], tmp);
                memset(tmp, 0, sizeof(*tmp));
                tmpLen = 0;
            } else if (tmpLen) {
                ret.pathType = ERROR;
                free(tmp);
                return ret;
            }
        } else if (pathname[i] == '/' && slashRead) {
            continue;
        } else {
            slashRead = 0;
            tmp[tmpLen++] = pathname[i];
        }
    }

    free(tmp);
    return ret;
}


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
    /* Initializing root directory */
    file *root = malloc(sizeof(*root));
    root->isDirectory = 1;
    root->fileName = NULL;
    root->nextFile = NULL;
    root->haveChild = 0;
    root->firstChildFile = NULL;

}
