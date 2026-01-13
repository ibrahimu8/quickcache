#ifndef HASH_H
#define HASH_H

#include <stddef.h>

#define HASH_SIZE 32
#define HASH_HEX_SIZE 65

typedef unsigned char hash_t[HASH_SIZE];

int hash_file(const char *path, hash_t out);
int hash_data(const void *data, size_t len, hash_t out);
void hash_to_hex(const hash_t hash, char *hex);
int hash_combine(const hash_t h1, const hash_t h2, hash_t out);

#endif
