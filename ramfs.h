#include <stdint.h>
#include <string.h>//adding this header to make the project compatible with macOS

#define O_APPEND 02000
#define O_CREAT 0100
#define O_TRUNC 01000
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

//To make the project compatible with macOS
//typedef int64_t ssize_t;
//typedef uint64_t size_t;
typedef int64_t off_t;

int ropen(const char *pathname, int flags);

int rclose(int fd);

ssize_t rwrite(int fd, const void *buf, size_t count);

ssize_t rread(int fd, void *buf, size_t count);

off_t rseek(int fd, off_t offset, int whence);

int rmkdir(const char *pathname);

int rrmdir(const char *pathname);

int runlink(const char *pathname);

void init_ramfs();
