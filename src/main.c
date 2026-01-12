#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hash.h"
#include "cache.h"
#include "exec.h"
#include "utils.h"
#include "stats.h"
#include "clean.h"
#include "metadata.h"
#include "config.h"
#include "network.h"

#define DEFAULT_CACHE_LIMIT (1024ULL * 1024 * 1024)

typedef struct {
    char *input_file;
    char *output_file;
    char *compiler;
} compile_info_t;

static void cleanup_handler(int sig) {
    (void)sig;
    cache_shutdown();
    metadata_close();
    exit(0);
}

int parse_args(int argc, char **argv, compile_info_t *info) {
    info->input_file = NULL;
    info->output_file = NULL;
    info->compiler = argv[0];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            info->output_file = argv[i + 1];
            i++;
        } else if (argv[i][0] != '-') {
            const char *ext = strrchr(argv[i], '.');
            if (ext && (strcmp(ext, ".c") == 0 ||
                       strcmp(ext, ".cpp") == 0 ||
                       strcmp(ext, ".cc") == 0 ||
                       strcmp(ext, ".cxx") == 0)) {
                info->input_file = argv[i];
            }
        }
    }

    if (!info->input_file) {
        return -1;
    }

    if (!info->output_file) {
        char *base = strdup(info->input_file);
        char *dot = strrchr(base, '.');
        if (dot) *dot = '\0';
        static char default_out[256];
        snprintf(default_out, sizeof(default_out), "%s.o", base);
        info->output_file = default_out;
        free(base);
    }

    return 0;
}

void build_command_string(int argc, char **argv, char *buf, size_t len) {
    buf[0] = '\0';
    for (int i = 0; i < argc; i++) {
        strncat(buf, argv[i], len - strlen(buf) - 1);
        if (i < argc - 1) {
            strncat(buf, " ", len - strlen(buf) - 1);
        }
    }
}

void print_usage(void) {
    printf("BuildCache - Distributed Compiler Cache\n\n");
    printf("Usage:\n");
    printf("  quickcache <compiler> <args...>     Run compiler with caching\n");
    printf("  quickcache --stats                  Show cache statistics\n");
    printf("  quickcache --clean [days]           Clean cache entries\n");
    printf("  quickcache --limit <size_mb>        Enforce cache size limit\n");
    printf("  quickcache --config                 Create example config file\n");
    printf("  quickcache --test-remote            Test remote cache connection\n\n");
    printf("Configuration:\n");
    printf("  Edit ~/.quickcache/config to enable remote caching\n");
    printf("  Example:\n");
    printf("    remote_url=http://your-cache-server:8080\n");
    printf("    auth_token=your-secret-token\n");
    printf("    async_upload=true\n\n");
}

int test_remote_connection(void) {
    if (cache_init() == -1) {
        fprintf(stderr, "Failed to initialize cache\n");
        return 1;
    }

    config_load();
    const quickcache_config_t *cfg = config_get();

    if (!cfg->remote_enabled) {
        printf("Remote cache is not enabled.\n");
        printf("Edit ~/.quickcache/config to configure remote cache.\n");
        cache_shutdown();
        metadata_close();
        return 1;
    }

    printf("Testing connection to: %s\n", cfg->remote_url);
    
    // Create a test hash
    hash_t test_key;
    const char *test_data = "quickcache-test-connection";
    hash_data(test_data, strlen(test_data), test_key);
    
    char hex[HASH_HEX_SIZE];
    hash_to_hex(test_key, hex);
    
    printf("Test hash: %s\n", hex);
    printf("Checking if remote server is accessible...\n");
    
    int exists = network_check_exists(test_key);
    
    if (exists) {
        printf("✓ Remote cache is accessible and responding!\n");
    } else {
        printf("✗ Could not reach remote cache or object doesn't exist\n");
        printf("  (This is normal if the test object hasn't been uploaded yet)\n");
    }

    cache_shutdown();
    metadata_close();
    return 0;
}

int main(int argc, char **argv) {
    // Setup signal handlers for clean shutdown
    signal(SIGINT, cleanup_handler);
    signal(SIGTERM, cleanup_handler);

    if (argc < 2) {
        print_usage();
        return 1;
    }

    // Load configuration
    config_load();

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "--config") == 0) {
        if (cache_init() == -1) {
            fprintf(stderr, "Failed to initialize cache\n");
            return 1;
        }
        int ret = config_create_example();
        cache_shutdown();
        metadata_close();
        return ret;
    }

    if (strcmp(argv[1], "--test-remote") == 0) {
        return test_remote_connection();
    }

    if (strcmp(argv[1], "--stats") == 0) {
        if (cache_init() == -1) {
            fprintf(stderr, "Failed to initialize cache\n");
            return 1;
        }
        stats_print();
        
        const quickcache_config_t *cfg = config_get();
        if (cfg->remote_enabled) {
            printf("\nRemote cache: ENABLED\n");
            printf("Server URL:   %s\n", cfg->remote_url);
            printf("Async upload: %s\n", cfg->async_upload ? "YES" : "NO");
        } else {
            printf("\nRemote cache: DISABLED\n");
        }
        
        cache_shutdown();
        metadata_close();
        return 0;
    }

    if (strcmp(argv[1], "--clean") == 0) {
        if (cache_init() == -1) {
            fprintf(stderr, "Failed to initialize cache\n");
            return 1;
        }

        if (argc == 2) {
            cache_clean_all();
        } else {
            int days = atoi(argv[2]);
            cache_clean_old(days);
        }

        cache_shutdown();
        metadata_close();
        return 0;
    }

    if (strcmp(argv[1], "--limit") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: quickcache --limit <size_in_mb>\n");
            return 1;
        }

        if (cache_init() == -1) {
            fprintf(stderr, "Failed to initialize cache\n");
            return 1;
        }

        size_t limit_mb = atoi(argv[2]);
        cache_enforce_limit(limit_mb * 1024 * 1024);

        cache_shutdown();
        metadata_close();
        return 0;
    }

    if (cache_init() == -1) {
        fprintf(stderr, "Failed to initialize cache\n");
        return 1;
    }

    compile_info_t info;
    if (parse_args(argc - 1, argv + 1, &info) == -1) {
        int ret = execute_compiler(argv + 1);
        cache_shutdown();
        metadata_close();
        return ret;
    }

    hash_t input_hash, cmd_hash, cache_key;

    if (hash_file(info.input_file, input_hash) == -1) {
        fprintf(stderr, "Failed to hash input file: %s\n", info.input_file);
        cache_shutdown();
        metadata_close();
        return 1;
    }

    char cmd_str[8192];
    build_command_string(argc - 1, argv + 1, cmd_str, sizeof(cmd_str));
    hash_data(cmd_str, strlen(cmd_str), cmd_hash);

    hash_combine(input_hash, cmd_hash, cache_key);

    char key_hex[HASH_HEX_SIZE];
    hash_to_hex(cache_key, key_hex);

    if (cache_lookup(cache_key, info.output_file) == 0) {
        printf("[quickcache] HIT %s\n", key_hex);
        cache_enforce_limit(DEFAULT_CACHE_LIMIT);
        cache_shutdown();
        metadata_close();
        return 0;
    }

    printf("[quickcache] MISS %s\n", key_hex);

    int ret = execute_compiler(argv + 1);

    if (ret == 0 && file_exists(info.output_file)) {
        if (cache_store(cache_key, info.output_file) == -1) {
            fprintf(stderr, "[quickcache] Warning: failed to store in cache\n");
        }
        cache_enforce_limit(DEFAULT_CACHE_LIMIT);
    }

    cache_shutdown();
    metadata_close();
    return ret;
}
