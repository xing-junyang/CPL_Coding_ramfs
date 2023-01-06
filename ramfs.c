#include "ramfs.h"
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

const int MAX_FILE_HANDLE = 65536;

/* To store all file and directories */
typedef struct File {
    /* Common properties */
    bool isDirectory;
    char *fileName;
    /* Directory properties */
    bool haveChild;
    struct File *firstChildFile;
    struct File *fatherFile;
    struct File *prevFile;
    struct File *nextFile;
    /* File properties */
    int fileSize;
    void *fileContent;
} file;

typedef struct FileHandle {
    bool rd;
    bool wr;
    bool isDirectory;
    file *targetFile;
    int offset;
} handle;
handle *handleMap[MAX_FILE_HANDLE];

typedef struct Path {
    enum {
        FILE, DIRECTORY, ERROR
    } pathType;
    char **name;
    int nameCount;
} path;

file *root;

bool checkNameValidity(const char *str) {
    int len = (int) strlen(str);
    for (int i = 0; i < len; i++) {
        if (!(isdigit(str[i]) || isalpha(str[i]) || str[i] == '.')) {
            return false;
        }
    }
    return true;
}

void destroyPath(path *src) {
    for (int i = 0; i < src->nameCount; i++) {
        free(src->name[i]);
    }
    free(src->name);
    free(src);
}

path *analyzePath(const char *pathname) {
    path *ret = malloc(sizeof(*ret));
    int len = (int) strlen(pathname);
    if (pathname[0] != '/') {
        ret->pathType = ERROR;
        return ret;
    }

    ret->name = malloc(sizeof(*ret->name) * 1024);
    ret->nameCount = 0;

    if (pathname[len - 1] == '/') {
        ret->pathType = DIRECTORY;
    } else {
        ret->pathType = FILE;
    }

    char *tmp = malloc(sizeof(char) * 1024);
    memset(tmp, 0, sizeof(*tmp));
    int tmpLen = 0;
    bool slashRead = 0;

    for (int i = 0; i < len; i++) {
        if ((pathname[i] == '/' && (!slashRead)) || i == len - 1) {
            slashRead = 1;
            if (checkNameValidity(tmp) && tmpLen) {
                ret->name[ret->nameCount++] = malloc(sizeof(char) * 1024);
                strcpy(ret->name[ret->nameCount], tmp);
                memset(tmp, 0, sizeof(*tmp));
                tmpLen = 0;
            } else if (tmpLen) {
                ret->pathType = ERROR;
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

    if (!ret->nameCount) {
        ret->pathType = ERROR;
        free(tmp);
        return ret;
    }

    free(tmp);
    return ret;
}

int findEmptyHandle() {
    for (int i = 0; i < MAX_FILE_HANDLE; i++) {
        if (handleMap[i] == NULL) {
            return i;
        }
    }
}

file *findFile(const file *directory, const char *name) {
    if (!directory->haveChild) {
        return NULL;
    }
    file *now = directory->firstChildFile;
    while (now->nextFile != NULL) {
        if (!strcmp(now->fileName, name)) {
            return now;
        }
        now = now->nextFile;
    }
    return NULL;
}

file *createFile(file *directory, char *fileName, bool isDirectory) {
    if (!directory->isDirectory) {
        return NULL;
    }
    file *ret = malloc(sizeof(*ret));
    directory->haveChild = true;
    ret->prevFile = NULL;
    ret->nextFile = directory->firstChildFile;
    ret->fatherFile = directory;
    ret->isDirectory = isDirectory;
    ret->haveChild = false;
    ret->firstChildFile = NULL;
    ret->fileName = fileName;
    ret->fileSize = 0;
    ret->fileContent = NULL;
    directory->firstChildFile->prevFile = ret;
    directory->firstChildFile = ret;
    return ret;
}

int ropen(const char *pathname, int flags) {
    path *nowPath = analyzePath(pathname);
    if (nowPath->pathType == ERROR) {
        destroyPath(nowPath);
        return -1;
    }

    file *target = root;

    bool isExistFile = 0;

    if (nowPath->pathType == DIRECTORY) {
        for (int i = 0; i < nowPath->nameCount; i++) {
            target = findFile(target, nowPath->name[i]);
            if (target == NULL) {
                destroyPath(nowPath);
                return -1;
            }
        }
    } else if (nowPath->pathType == FILE) {
        for (int i = 0; i < nowPath->nameCount - 1; i++) {
            target = findFile(target, nowPath->name[i]);
            if (target == NULL) {
                destroyPath(nowPath);
                return -1;
            }
        }
        if ((flags & O_CREAT) && (findFile(target, nowPath->name[nowPath->nameCount - 1]) == NULL)) {
            target = createFile(target, nowPath->name[nowPath->nameCount - 1], 0);
        } else if (findFile(target, nowPath->name[nowPath->nameCount - 1]) == NULL) {
            destroyPath(nowPath);
            return -1;
        } else {
            target = findFile(target, nowPath->name[nowPath->nameCount - 1]);
            isExistFile = 1;
        }
    }

    int ret = findEmptyHandle();
    handleMap[ret] = malloc(sizeof(*(handleMap[ret])));

    handleMap[ret]->targetFile = target;

    handleMap[ret]->rd = 1;
    handleMap[ret]->wr = 0;
    if (flags & O_WRONLY) {
        handleMap[ret]->rd = 0;
        handleMap[ret]->wr = 1;
    }
    if (flags & O_RDWR) {
        handleMap[ret]->rd = 1;
        handleMap[ret]->wr = 1;
    }

    if (flags & O_TRUNC && isExistFile && handleMap[ret]->wr == 1) {
        memset(target->fileContent, 0, target->fileSize);
    }

    if (flags & O_APPEND) {
        handleMap[ret]->offset = target->fileSize - 1;
    } else {
        handleMap[ret]->offset = 0;
    }

    destroyPath(nowPath);
    return ret;
}

int rclose(int fd) {
    if (handleMap[fd] == NULL) {
        return -1;
    } else {
        free(handleMap[fd]);
        return 0;
    }
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    // TODO();
}

ssize_t rread(int fd, void *buf, size_t count) {
    // TODO();
}

off_t rseek(int fd, off_t offset, int whence) {
    // TODO();
    if
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
    root = malloc(sizeof(*root));
    root->isDirectory = 1;
    root->fileName = NULL;
    root->fatherFile = NULL;
    root->prevFile = NULL;
    root->nextFile = NULL;
    root->haveChild = false;
    root->firstChildFile = NULL;

}
