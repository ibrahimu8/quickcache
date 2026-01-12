#define _POSIX_C_SOURCE 200809L
#include "metadata.h"
#include "cache.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>
#include <time.h>

static sqlite3 *db = NULL;

void get_db_path(char *buf, size_t len) {
    char home[4096];
    get_home_dir(home, sizeof(home));
    snprintf(buf, len, "%s/%s/cache.db", home, CACHE_DIR_NAME);
}

int metadata_init(void) {
    char db_path[4096];
    get_db_path(db_path, sizeof(db_path));

    /* Retry logic for concurrent access */
    int retries = 10;
    int delay_ms = 10;
    
    for (int i = 0; i < retries; i++) {
        int rc = sqlite3_open(db_path, &db);
        
        if (rc == SQLITE_OK) {
            /* Enable WAL mode for better concurrent access */
            char *err = NULL;
            sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, &err);
            if (err) sqlite3_free(err);
            
            /* Set busy timeout */
            sqlite3_busy_timeout(db, 5000);
            break;
        }
        
        /* If database is locked, wait and retry */
        if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED) {
            struct timespec ts = {0, delay_ms * 1000000};
            nanosleep(&ts, NULL);
            delay_ms *= 2; /* Exponential backoff */
            continue;
        }
        
        /* Other error, fail */
        return -1;
    }
    
    if (!db) return -1;

    const char *schema =
        "CREATE TABLE IF NOT EXISTS cache_entries ("
        "hash TEXT PRIMARY KEY,"
        "path TEXT NOT NULL,"
        "size INTEGER NOT NULL,"
        "compressed_size INTEGER NOT NULL,"
        "created INTEGER NOT NULL,"
        "accessed INTEGER NOT NULL,"
        "compressed INTEGER NOT NULL"
        ");";

    char *err = NULL;
    if (sqlite3_exec(db, schema, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(err);
        return -1;
    }

    const char *index = "CREATE INDEX IF NOT EXISTS idx_accessed ON cache_entries(accessed);";
    if (sqlite3_exec(db, index, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(err);
        return -1;
    }

    return 0;
}

int metadata_add(const char *hash, const char *path, size_t size, size_t compressed_size, int compressed) {
    if (!db) return -1;

    const char *sql = "INSERT OR REPLACE INTO cache_entries "
                      "(hash, path, size, compressed_size, created, accessed, compressed) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }

    time_t now = time(NULL);

    sqlite3_bind_text(stmt, 1, hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)size);
    sqlite3_bind_int64(stmt, 4, (sqlite3_int64)compressed_size);
    sqlite3_bind_int64(stmt, 5, (sqlite3_int64)now);
    sqlite3_bind_int64(stmt, 6, (sqlite3_int64)now);
    sqlite3_bind_int(stmt, 7, compressed);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int metadata_get(const char *hash, cache_entry_t *entry) {
    if (!db) return -1;

    const char *sql = "SELECT * FROM cache_entries WHERE hash = ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, hash, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }

    strncpy(entry->hash, (const char *)sqlite3_column_text(stmt, 0), HASH_HEX_SIZE);
    strncpy(entry->path, (const char *)sqlite3_column_text(stmt, 1), 4096);
    entry->size = sqlite3_column_int64(stmt, 2);
    entry->compressed_size = sqlite3_column_int64(stmt, 3);
    entry->created = sqlite3_column_int64(stmt, 4);
    entry->accessed = sqlite3_column_int64(stmt, 5);
    entry->compressed = sqlite3_column_int(stmt, 6);

    sqlite3_finalize(stmt);
    return 0;
}

int metadata_update_access(const char *hash) {
    if (!db) return -1;

    const char *sql = "UPDATE cache_entries SET accessed = ? WHERE hash = ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }

    time_t now = time(NULL);
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)now);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int metadata_delete(const char *hash) {
    if (!db) return -1;

    const char *sql = "DELETE FROM cache_entries WHERE hash = ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, hash, -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE ? 0 : -1;
}

uint64_t metadata_total_size(void) {
    if (!db) return 0;

    const char *sql = "SELECT SUM(compressed_size) FROM cache_entries;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    uint64_t total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return total;
}

int metadata_get_lru_entries(cache_entry_t **entries, int *count, size_t limit) {
    if (!db) return -1;

    const char *sql = "SELECT * FROM cache_entries ORDER BY accessed ASC;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }

    int capacity = 100;
    *entries = malloc(capacity * sizeof(cache_entry_t));
    *count = 0;

    uint64_t total = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            *entries = realloc(*entries, capacity * sizeof(cache_entry_t));
        }

        cache_entry_t *entry = &(*entries)[*count];
        strncpy(entry->hash, (const char *)sqlite3_column_text(stmt, 0), HASH_HEX_SIZE);
        strncpy(entry->path, (const char *)sqlite3_column_text(stmt, 1), 4096);
        entry->size = sqlite3_column_int64(stmt, 2);
        entry->compressed_size = sqlite3_column_int64(stmt, 3);
        entry->created = sqlite3_column_int64(stmt, 4);
        entry->accessed = sqlite3_column_int64(stmt, 5);
        entry->compressed = sqlite3_column_int(stmt, 6);

        total += entry->compressed_size;
        (*count)++;

        if (total > limit) {
            break;
        }
    }

    sqlite3_finalize(stmt);
    return 0;
}

int metadata_get_old_entries(int days, cache_entry_t **entries, int *count) {
    if (!db) return -1;

    time_t cutoff = time(NULL) - (days * 24 * 3600);

    const char *sql = "SELECT hash, size FROM cache_entries WHERE accessed < ? ORDER BY accessed ASC;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)cutoff);

    *entries = NULL;
    *count = 0;
    int capacity = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity = capacity == 0 ? 64 : capacity * 2;
            *entries = realloc(*entries, capacity * sizeof(cache_entry_t));
        }

        const char *hash = (const char *)sqlite3_column_text(stmt, 0);
        size_t size = (size_t)sqlite3_column_int64(stmt, 1);

        strncpy((*entries)[*count].hash, hash, sizeof((*entries)[*count].hash) - 1);
        (*entries)[*count].size = size;
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int metadata_get_all_entries(cache_entry_t **entries, int *count) {
    if (!db) return -1;

    const char *sql = "SELECT hash, size FROM cache_entries ORDER BY accessed ASC;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }

    *entries = NULL;
    *count = 0;
    int capacity = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity = capacity == 0 ? 64 : capacity * 2;
            *entries = realloc(*entries, capacity * sizeof(cache_entry_t));
        }

        const char *hash = (const char *)sqlite3_column_text(stmt, 0);
        size_t size = (size_t)sqlite3_column_int64(stmt, 1);

        strncpy((*entries)[*count].hash, hash, sizeof((*entries)[*count].hash) - 1);
        (*entries)[*count].size = size;
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

void metadata_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}
