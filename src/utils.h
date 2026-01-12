#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

int file_exists(const char *path);
int make_dirs(const char *path);
int copy_file(const char *src, const char *dst);
char* read_file(const char *path, size_t *len);
int write_file(const char *path, const void *data, size_t len);
void get_home_dir(char *buf, size_t len);

#endif
