/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#include <cstdlib>
#include <iostream>
#include <modules>

extern "C"
{
    void FileSystem_start();
    
    // placeholder untill complete
    int file_size(int fdesc);
    int close(int fdesc);
    int open(const char *str, int how);
    int    read(int fdesc, char *buf, int count);
    int    readdir(struct dirstuff *dirp, const char **name, long *flags, long *time);
    struct dirstuff * opendir(const char *path);
}

void FileSystem_start()
{
}

int file_size(int fdesc)
{
    return -1;  // unsupported
}

int close(int fdesc)
{
    return -1;
}

int open(const char *str, int how)
{
    return -1;
}
int    read(int fdesc, char *buf, int count)
{
    return -1;
}

int    readdir(struct dirstuff *dirp, const char **name, long *flags, long *time)
{
    return -1;
}

struct dirstuff * opendir(const char *path)
{
    return NULL;
}

