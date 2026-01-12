#ifndef STATS_H
#define STATS_H

#include <stdint.h>
#include <time.h>

typedef struct {
    uint64_t hits;
    uint64_t misses;
    uint64_t bytes_saved;
    uint64_t total_lookups;
    time_t created;
    time_t last_updated;
} stats_t;

int stats_init(void);
int stats_load(stats_t *stats);
int stats_save(const stats_t *stats);
void stats_record_hit(size_t bytes);
void stats_record_miss(void);
void stats_print(void);

#endif
