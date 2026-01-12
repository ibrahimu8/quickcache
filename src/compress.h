#ifndef COMPRESS_H
#define COMPRESS_H

#include <stddef.h>

int compress_file(const char *src, const char *dst, size_t *compressed_size);
int decompress_file(const char *src, const char *dst);

#endif
