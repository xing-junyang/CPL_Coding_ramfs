# CPL_Coding_ramfs

> This is the **Final Project** of **C Programming Language** in my freshman year.

This is a simple file system on RAM, which means it is **volatile** and will be lost after the system is terminated. It
is implemented in C and simulates the classical `UNIX` file system.

I have implemented the following funcs:

```c
int ropen(const char *pathname, int flags);// open a file

int rclose(int fd);// close a file

ssize_t rwrite(int fd, const void *buf, size_t count);// write to a file

ssize_t rread(int fd, void *buf, size_t count);// read from a file

off_t rseek(int fd, off_t offset, int whence);// seek a file

int rmkdir(const char *pathname);// make a directory

int rrmdir(const char *pathname);// remove a directory

int runlink(const char *pathname);// remove a file

void init_ramfs();// initialize the file system
```

Try `make run` to compile and run!
