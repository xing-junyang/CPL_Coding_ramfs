#include "ramfs.h"
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <printf.h>

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
    size_t fileSize;
    void *fileContent;
} file;

typedef struct FileHandle {
    bool rd;
    bool wr;
    bool isDirectory;
    file *targetFile;
    size_t offset;
} handle;
handle *handleMap[65536];

typedef struct Path {
    enum {
        FL, ERROR
    } pathType;
    char **name;
    int nameCount;
} path;

file *root;

size_t max(size_t a, size_t b) { return a > b ? a : b; }

size_t min(size_t a, size_t b) { return a < b ? a : b; }

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
//    free(src->name);
    free(src);
}

path *analyzePath(const char *pathname) {
    path *ret = malloc(sizeof(*ret));
    ret->name = malloc(sizeof(*(ret->name)) * 1025);
    ret->nameCount = 0;
    ret->pathType = FL;
    int len = (int) strlen(pathname);
    if (pathname[0] != '/') {
        ret->pathType = ERROR;
        return ret;
    }
    char *tmp = malloc(sizeof(char) * 1025);
    memset(tmp, 0, 1025);
    int tmpLen = 0;
    bool slashRead = 0;
    for (int i = 0; i < len; i++) {
        if (pathname[i] == '/' && (!slashRead)) {
            slashRead = 1;
            if (tmpLen && checkNameValidity(tmp)) {
                ret->name[ret->nameCount] = malloc(sizeof(char) * 1025);
                strcpy(ret->name[ret->nameCount], tmp);
                ret->nameCount++;
                memset(tmp, 0, 1025);
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
    if (pathname[len - 1] != '/') {
        ret->name[ret->nameCount] = malloc(sizeof(char) * 1025);
        strcpy(ret->name[ret->nameCount], tmp);
        ret->nameCount++;
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
    return -1;//impossible.
}

file *findFile(const file *directory, const char *name) {
    if (!directory->haveChild) {
        return NULL;
    }
    file *now = directory->firstChildFile;
    do {
        if (!strcmp(now->fileName, name)) {
            return now;
        }
        now = now->nextFile;
    } while (now != NULL);
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
    if (directory->firstChildFile != NULL) {
        directory->firstChildFile->prevFile = ret;
    }
    directory->firstChildFile = ret;
    return ret;
}

int removeFile(file *target) {
    if (target == NULL) {
        return -1;
    }

    file *directory = target->fatherFile;

    if (directory->firstChildFile == target) {
        directory->firstChildFile = target->nextFile;
        if (target->nextFile == NULL) {
            directory->haveChild = false;
        } else {
            directory->haveChild = true;
            target->nextFile->prevFile = directory;
        }
        free(target);
        return 0;
    } else {
        target->prevFile->nextFile = target->nextFile;
        if (target->nextFile != NULL) {
            target->nextFile->prevFile = target->prevFile;
        }
        free(target);
        return 0;
    }
}

int ropen(const char *pathname, int flags) {
    path *nowPath = analyzePath(pathname);
    if (nowPath->pathType == ERROR) {
        destroyPath(nowPath);
        return -1;
    }

    file *target = root;

    bool newCreatedFile = 0;

    if (flags & O_CREAT) {
        for (int i = 0; i < nowPath->nameCount - 1; i++) {
            target = findFile(target, nowPath->name[i]);
            if (target == NULL) {
                destroyPath(nowPath);
                return -1;
            }
        }
        if (findFile(target, nowPath->name[nowPath->nameCount - 1]) == NULL) {
            target = createFile(target, nowPath->name[nowPath->nameCount - 1], 0);
            newCreatedFile = 1;
        } else {
            target = findFile(target, nowPath->name[nowPath->nameCount - 1]);
        }
    } else {
        for (int i = 0; i < nowPath->nameCount; i++) {
            target = findFile(target, nowPath->name[i]);
            if (target == NULL) {
                destroyPath(nowPath);
                return -1;
            }
        }
    }

    int ret = findEmptyHandle();
    handleMap[ret] = malloc(sizeof(*(handleMap[ret])));

    handleMap[ret]->targetFile = target;
    handleMap[ret]->isDirectory = target->isDirectory;

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

    if (flags & O_TRUNC && (!newCreatedFile) && handleMap[ret]->wr == 1 && (!handleMap[ret]->isDirectory)) {
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
        handleMap[fd] = NULL;
        return 0;
    }
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    if (handleMap[fd] == NULL || handleMap[fd]->isDirectory || (!handleMap[fd]->rd)) {
        return -1;
    }

    file *target = handleMap[fd]->targetFile;

    size_t fileSizeUpdate = max(target->fileSize, handleMap[fd]->offset + count);
    void *fileContentUpdate = malloc(fileSizeUpdate);
    memset(fileContentUpdate, 0, fileSizeUpdate);

    if (target->fileContent != NULL) {
        memcpy(fileContentUpdate, target->fileContent, target->fileSize);
    }
    memcpy(fileContentUpdate + handleMap[fd]->offset, buf, count);
    handleMap[fd]->offset += count;

    target->fileSize = fileSizeUpdate;
    free(target->fileContent);
    target->fileContent = fileContentUpdate;

    return count;
}

ssize_t rread(int fd, void *buf, size_t count) {
    if (handleMap[fd] == NULL || handleMap[fd]->isDirectory || (!handleMap[fd]->rd)) {
        return -1;
    }

    file const *target = handleMap[fd]->targetFile;

    size_t readSize = min(count, max(0, target->fileSize - handleMap[fd]->offset));
    memcpy(buf, target->fileContent + handleMap[fd]->offset, readSize);
    handleMap[fd]->offset += count;

    return readSize;
}

off_t rseek(int fd, off_t offset, int whence) {
    if (handleMap[fd] == NULL) {
        return -1;
    }
    switch (whence) {
        case SEEK_SET:
            if (offset < 0) {
                return -1;
            }
            handleMap[fd]->offset = offset;
            break;
        case SEEK_CUR:
            if ((off_t) handleMap[fd]->offset + offset < 0) {
                return -1;
            }
            handleMap[fd]->offset += offset;
            break;
        case SEEK_END:
            if ((off_t) handleMap[fd]->targetFile->fileSize + offset < 0) {
                return -1;
            }
            handleMap[fd]->offset = handleMap[fd]->targetFile->fileSize + offset;
            break;
        default:
            break;
    }
    return (off_t) handleMap[fd]->offset;
}

int rmkdir(const char *pathname) {
    path *nowPath;
    nowPath = analyzePath(pathname);
//    if(!strcmp(pathname,"/a/b")){
//        nowPath= malloc(sizeof (*nowPath));
//        nowPath->pathType=FL;
//        nowPath->nameCount=2;
//        nowPath->name = malloc(sizeof (*nowPath->name)*1025);
//        nowPath->name[0]= malloc(1025);
//        nowPath->name[1]= malloc(1025);
//        strcpy(nowPath->name[0],"a");
//        strcpy(nowPath->name[1],"b");
//        printf("%s",root->firstChildFile->fileName);
//    }else{
//        nowPath= analyzePath(pathname);
//    }


    if (nowPath->pathType == ERROR) {
        destroyPath(nowPath);
        return -1;
    }

    file *target = root;

    for (int i = 0; i < nowPath->nameCount - 1; i++) {
        target = findFile(target, nowPath->name[i]);
        if (target == NULL) {
            destroyPath(nowPath);
            return -1;
        }
    }

    if (findFile(target, nowPath->name[nowPath->nameCount - 1]) != NULL) {
        destroyPath(nowPath);
        return -1;
    }

    if (createFile(target, nowPath->name[nowPath->nameCount - 1], 1) != NULL) {
        destroyPath(nowPath);
        return 0;
    }

    destroyPath(nowPath);
    return -1;
}

int rrmdir(const char *pathname) {
    path *nowPath = analyzePath(pathname);

    if (nowPath->pathType == ERROR) {
        destroyPath(nowPath);
        return -1;
    }

    file *target = root;

    for (int i = 0; i < nowPath->nameCount; i++) {
        target = findFile(target, nowPath->name[i]);
        if (target == NULL) {
            destroyPath(nowPath);
            return -1;
        }
    }

    if (target->haveChild || (!target->isDirectory)) {
        destroyPath(nowPath);
        return -1;
    }

    return removeFile(target);
}

int runlink(const char *pathname) {
    path *nowPath = analyzePath(pathname);

    if (nowPath->pathType == ERROR) {
        destroyPath(nowPath);
        return -1;
    }

    file *target = root;

    for (int i = 0; i < nowPath->nameCount; i++) {
        target = findFile(target, nowPath->name[i]);
        if (target == NULL) {
            destroyPath(nowPath);
            return -1;
        }
    }

    if (target->isDirectory) {
        destroyPath(nowPath);
        return -1;
    }
    destroyPath(nowPath);
    return removeFile(target);
}

void init_ramfs() {
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