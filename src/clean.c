#include "clean.h"
#include "cache.h"
#include "metadata.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

static int remove_cache_object(const char *hash) {
    char path[4096];
    char cache_dir[4096];
    
    cache_get_base_dir(cache_dir, sizeof(cache_dir));
    snprintf(path, sizeof(path), "%s/objects/%.2s/%s", cache_dir, hash, hash + 2);
    
    if (unlink(path) == 0) {
        metadata_delete(hash);
        return 0;
    }
    
    return -1;
}

int cache_clean_all(void) {
    char objects_dir[4096];
    cache_get_base_dir(objects_dir, sizeof(objects_dir));
    strcat(objects_dir, "/objects");
    
    DIR *d = opendir(objects_dir);
    if (!d) return -1;
    
    int removed = 0;
    struct dirent *entry;
    
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char subdir[4096];
        snprintf(subdir, sizeof(subdir), "%s/%s", objects_dir, entry->d_name);
        
        DIR *sd = opendir(subdir);
        if (!sd) continue;
        
        struct dirent *file;
        while ((file = readdir(sd)) != NULL) {
            if (file->d_name[0] == '.') continue;
            
            char filepath[4096];
            snprintf(filepath, sizeof(filepath), "%s/%s", subdir, file->d_name);
            
            char hash[HASH_HEX_SIZE];
            snprintf(hash, sizeof(hash), "%s%s", entry->d_name, file->d_name);
            
            if (unlink(filepath) == 0) {
                metadata_delete(hash);
                removed++;
            }
        }
        closedir(sd);
        rmdir(subdir);
    }
    closedir(d);
    
    printf("Removed %d cache entries\n", removed);
    return 0;
}

int cache_clean_old(int days) {
    time_t now = time(NULL);
    time_t cutoff = now - (days * 86400);
    
    char objects_dir[4096];
    cache_get_base_dir(objects_dir, sizeof(objects_dir));
    strcat(objects_dir, "/objects");
    
    DIR *d = opendir(objects_dir);
    if (!d) return -1;
    
    int removed = 0;
    struct dirent *entry;
    
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char subdir[4096];
        snprintf(subdir, sizeof(subdir), "%s/%s", objects_dir, entry->d_name);
        
        DIR *sd = opendir(subdir);
        if (!sd) continue;
        
        struct dirent *file;
        while ((file = readdir(sd)) != NULL) {
            if (file->d_name[0] == '.') continue;
            
            char filepath[4096];
            snprintf(filepath, sizeof(filepath), "%s/%s", subdir, file->d_name);
            
            struct stat st;
            if (stat(filepath, &st) == 0 && st.st_mtime < cutoff) {
                char hash[HASH_HEX_SIZE];
                snprintf(hash, sizeof(hash), "%s%s", entry->d_name, file->d_name);
                
                if (unlink(filepath) == 0) {
                    metadata_delete(hash);
                    removed++;
                }
            }
        }
        closedir(sd);
    }
    closedir(d);
    
    printf("Removed %d old cache entries\n", removed);
    return 0;
}

int cache_enforce_limit(size_t max_bytes) {
    uint64_t total = metadata_total_size();
    
    if (total <= max_bytes) {
        return 0;
    }
    
    size_t to_free = total - max_bytes;
    
    cache_entry_t *entries;
    int count;
    
    if (metadata_get_lru_entries(&entries, &count, total) != 0) {
        return -1;
    }
    
    size_t freed = 0;
    int removed = 0;
    
    for (int i = 0; i < count && freed < to_free; i++) {
        if (remove_cache_object(entries[i].hash) == 0) {
            freed += entries[i].compressed_size;
            removed++;
        }
    }
    
    free(entries);
    
    if (removed > 0) {
        printf("Evicted %d entries to enforce size limit (%.2f MB freed)\n", 
               removed, freed / (1024.0 * 1024.0));
    }
    
    return 0;
}
