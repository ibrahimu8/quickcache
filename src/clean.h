#include <stddef.h>
#ifndef CLEAN_H
#define CLEAN_H

int cache_clean_all(void);
int cache_clean_old(int days);
int cache_enforce_limit(size_t max_bytes);

#endif
