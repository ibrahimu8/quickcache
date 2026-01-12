#include "cache.h"
#include "utils.h"
#include "metadata.h"
#include "compress.h"
#include "stats.h"
#include "network.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

void cache_get_base_dir(char *buf, size_t len) {
    char home[4096];
    get_home_dir(home, sizeof(home));
    snprintf(buf, len, "%s/%s", home, CACHE_DIR_NAME);
}

int cache_init(void) {
    char cache_dir[4096];
    char objects_dir[4096];

    cache_get_base_dir(cache_dir, sizeof(cache_dir));
    snprintf(objects_dir, sizeof(objects_dir), "%s/objects", cache_dir);

    if (make_dirs(objects_dir) == -1) {
        return -1;
    }

    if (metadata_init() == -1) {
        return -1;
    }

    if (stats_init() == -1) {
        return -1;
    }

    if (network_init() == -1) {
        return -1;
    }

    return 0;
}

void cache_get_object_path(const hash_t key, char *buf, size_t len) {
    char hex[HASH_HEX_SIZE];
    char cache_dir[4096];

    hash_to_hex(key, hex);
    cache_get_base_dir(cache_dir, sizeof(cache_dir));

    snprintf(buf, len, "%s/objects/%.2s", cache_dir, hex);
    make_dirs(buf);

    snprintf(buf, len, "%s/objects/%.2s/%s", cache_dir, hex, hex + 2);
}

int cache_lookup(const hash_t key, const char *output_path) {
    char cache_path[4096];
    char hex[HASH_HEX_SIZE];

    hash_to_hex(key, hex);
    cache_get_object_path(key, cache_path, sizeof(cache_path));

    // L1 Cache: Try local first
    if (file_exists(cache_path)) {
        cache_entry_t entry;
        if (metadata_get(hex, &entry) == 0) {
            metadata_update_access(hex);

            if (entry.compressed) {
                if (decompress_file(cache_path, output_path) == 0) {
                    stats_record_hit(entry.size);
                    printf("[quickcache] LOCAL HIT\n");
                    return 0;
                }
                return -1;
            }
        }

        if (copy_file(cache_path, output_path) == 0) {
            struct stat st;
            if (stat(output_path, &st) == 0) {
                stats_record_hit(st.st_size);
            }
            printf("[quickcache] LOCAL HIT\n");
            return 0;
        }
    }

    // L2 Cache: Try remote
    printf("[quickcache] Checking remote cache...\n");
    if (network_get(key, output_path) == 0) {
        printf("[quickcache] REMOTE HIT\n");
        
        // Store in local cache for next time
        struct stat st;
        if (stat(output_path, &st) == 0) {
            size_t original_size = st.st_size;
            size_t compressed_size = 0;
            int compressed = 0;
            char cache_path_tmp[4096];
            
            snprintf(cache_path_tmp, sizeof(cache_path_tmp), "%s.tmp", cache_path);
            
            if (compress_file(output_path, cache_path_tmp, &compressed_size) == 0) {
                if (compressed_size < original_size * 0.9) {
                    rename(cache_path_tmp, cache_path);
                    compressed = 1;
                    metadata_add(hex, cache_path, original_size, compressed_size, 1);
                } else {
                    unlink(cache_path_tmp);
                }
            }
            
            if (!compressed) {
                if (copy_file(output_path, cache_path) == 0) {
                    compressed_size = original_size;
                    metadata_add(hex, cache_path, original_size, compressed_size, 0);
                }
            }
            
            stats_record_hit(original_size);
        }
        
        return 0;
    }

    stats_record_miss();
    return -1;
}

int cache_store(const hash_t key, const char *file_path) {
    char cache_path[4096];
    char cache_path_tmp[4096];
    char hex[HASH_HEX_SIZE];

    hash_to_hex(key, hex);
    cache_get_object_path(key, cache_path, sizeof(cache_path));
    snprintf(cache_path_tmp, sizeof(cache_path_tmp), "%s.tmp", cache_path);

    struct stat st;
    if (stat(file_path, &st) != 0) {
        return -1;
    }

    size_t original_size = st.st_size;
    size_t compressed_size = 0;
    int compressed = 0;

    // Store locally with compression
    if (compress_file(file_path, cache_path_tmp, &compressed_size) == 0) {
        if (compressed_size < original_size * 0.9) {
            rename(cache_path_tmp, cache_path);
            compressed = 1;
            metadata_add(hex, cache_path, original_size, compressed_size, 1);
        } else {
            unlink(cache_path_tmp);
        }
    }

    if (!compressed) {
        if (copy_file(file_path, cache_path) != 0) {
            return -1;
        }
        compressed_size = original_size;
        metadata_add(hex, cache_path, original_size, compressed_size, 0);
    }

    // Upload to remote cache (async)
    printf("[quickcache] Uploading to remote cache...\n");
    network_put_async(key, cache_path);

    return 0;
}

void cache_shutdown(void) {
    network_cleanup();
}
