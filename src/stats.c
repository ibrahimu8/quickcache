#include "stats.h"
#include "cache.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define STATS_FILE "stats.bin"

static void get_stats_path(char *buf, size_t len) {
    char cache_dir[4096];
    cache_get_base_dir(cache_dir, sizeof(cache_dir));
    snprintf(buf, len, "%s/%s", cache_dir, STATS_FILE);
}

int stats_init(void) {
    char path[4096];
    get_stats_path(path, sizeof(path));
    
    if (!file_exists(path)) {
        stats_t stats = {0};
        stats.created = time(NULL);
        stats.last_updated = time(NULL);
        return stats_save(&stats);
    }
    return 0;
}

int stats_load(stats_t *stats) {
    char path[4096];
    get_stats_path(path, sizeof(path));
    
    FILE *f = fopen(path, "rb");
    if (!f) {
        memset(stats, 0, sizeof(stats_t));
        stats->created = time(NULL);
        stats->last_updated = time(NULL);
        return -1;
    }
    
    size_t read = fread(stats, sizeof(stats_t), 1, f);
    fclose(f);
    return read == 1 ? 0 : -1;
}

int stats_save(const stats_t *stats) {
    char path[4096];
    get_stats_path(path, sizeof(path));
    
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    
    size_t written = fwrite(stats, sizeof(stats_t), 1, f);
    fclose(f);
    return written == 1 ? 0 : -1;
}

void stats_record_hit(size_t bytes) {
    stats_t stats;
    stats_load(&stats);
    stats.hits++;
    stats.total_lookups++;
    stats.bytes_saved += bytes;
    stats.last_updated = time(NULL);
    stats_save(&stats);
}

void stats_record_miss(void) {
    stats_t stats;
    stats_load(&stats);
    stats.misses++;
    stats.total_lookups++;
    stats.last_updated = time(NULL);
    stats_save(&stats);
}

void stats_print(void) {
    stats_t stats;
    if (stats_load(&stats) == -1) {
        printf("No statistics available\n");
        return;
    }
    
    double hit_rate = stats.total_lookups > 0 
        ? (100.0 * stats.hits / stats.total_lookups) 
        : 0.0;
    
    double mb_saved = stats.bytes_saved / (1024.0 * 1024.0);
    
    printf("BuildCache Statistics\n");
    printf("=====================\n");
    printf("Cache hits:     %lu\n", stats.hits);
    printf("Cache misses:   %lu\n", stats.misses);
    printf("Hit rate:       %.1f%%\n", hit_rate);
    printf("Data saved:     %.2f MB\n", mb_saved);
    
    time_t now = time(NULL);
    double days = difftime(now, stats.created) / 86400.0;
    printf("Cache age:      %.1f days\n", days);
}
