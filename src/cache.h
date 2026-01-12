#ifndef CACHE_H
#define CACHE_H

#include "hash.h"

#define CACHE_DIR_NAME ".quickcache"

int cache_init(void);
void cache_get_base_dir(char *buf, size_t len);
void cache_get_object_path(const hash_t key, char *buf, size_t len);
int cache_lookup(const hash_t key, const char *output_path);
int cache_store(const hash_t key, const char *file_path);
void cache_shutdown(void);

#endif
