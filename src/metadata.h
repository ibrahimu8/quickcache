#include <stdint.h>
#ifndef METADATA_H
#define METADATA_H

#include <stddef.h>
#include <time.h>
#include <stdint.h>
#include "hash.h"

typedef struct {
    char hash[HASH_HEX_SIZE];
    char path[4096];
    size_t size;
    size_t compressed_size;
    time_t created;
    time_t accessed;
    int compressed;
} cache_entry_t;

int metadata_init(void);
int metadata_add(const char *hash, const char *path, size_t size, size_t compressed_size, int compressed);
int metadata_get(const char *hash, cache_entry_t *entry);
int metadata_update_access(const char *hash);
int metadata_delete(const char *hash);
uint64_t metadata_total_size(void);
int metadata_get_lru_entries(cache_entry_t **entries, int *count, size_t limit);
void metadata_close(void);

#endif
