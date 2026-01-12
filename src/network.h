#ifndef NETWORK_H
#define NETWORK_H

#include "hash.h"

int network_init(void);
void network_cleanup(void);
int network_get(const hash_t key, const char *output_path);
int network_put(const hash_t key, const char *file_path);
void network_put_async(const hash_t key, const char *file_path);
int network_check_exists(const hash_t key);

#endif
