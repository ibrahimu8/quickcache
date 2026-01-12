#include "config.h"
#include "cache.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static quickcache_config_t global_config = {0};
static int config_loaded = 0;

/* Build ~/.quickcache/config path */
static void get_config_path(char *buf, size_t len) {
    char cache_dir[4096];
    cache_get_base_dir(cache_dir, sizeof(cache_dir));
    snprintf(buf, len, "%s/config", cache_dir);
}

/* Trim whitespace IN PLACE (leading + trailing) */
static void trim(char *s) {
    char *start = s;
    char *end;

    /* Leading whitespace */
    while (*start && isspace((unsigned char)*start))
        start++;

    if (start != s)
        memmove(s, start, strlen(start) + 1);

    /* Trailing whitespace */
    end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

/* Parse single config line */
static void parse_line(char *line) {
    char *eq;

    /* Strip newline */
    char *newline = strchr(line, '\n');
    if (newline) *newline = '\0';

    trim(line);

    /* Skip empty lines & comments */
    if (line[0] == '\0' || line[0] == '#')
        return;

    /* Split key=value */
    eq = strchr(line, '=');
    if (!eq)
        return;

    *eq = '\0';

    char *key = line;
    char *value = eq + 1;

    trim(key);
    trim(value);

    if (strcmp(key, "remote_url") == 0) {
        strncpy(global_config.remote_url, value,
                sizeof(global_config.remote_url) - 1);
        global_config.remote_enabled = 1;

    } else if (strcmp(key, "auth_token") == 0) {
        strncpy(global_config.auth_token, value,
                sizeof(global_config.auth_token) - 1);

    } else if (strcmp(key, "timeout") == 0) {
        global_config.timeout_seconds = atoi(value);

    } else if (strcmp(key, "async_upload") == 0) {
        global_config.async_upload =
            (strcmp(value, "true") == 0 ||
             strcmp(value, "1") == 0 ||
             strcmp(value, "yes") == 0 ||
             strcmp(value, "on") == 0);

    } else if (strcmp(key, "ignore_output_path") == 0) {
        global_config.ignore_output_path =
            (strcmp(value, "true") == 0 ||
             strcmp(value, "1") == 0 ||
             strcmp(value, "yes") == 0 ||
             strcmp(value, "on") == 0);
    }
}

/* Load config file */
int config_load(void) {
    if (config_loaded)
        return 0;

    /* Defaults */
    memset(&global_config, 0, sizeof(global_config));
    global_config.remote_enabled = 0;
    global_config.timeout_seconds = 10;
    global_config.async_upload = 1;
    global_config.ignore_output_path = 0;

    char config_path[4096];
    get_config_path(config_path, sizeof(config_path));

    FILE *f = fopen(config_path, "r");
    if (!f) {
        config_loaded = 1;
        return 0; /* Config optional */
    }

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        parse_line(line);
    }

    fclose(f);
    config_loaded = 1;
    return 0;
}

/* Access config */
const quickcache_config_t* config_get(void) {
    if (!config_loaded)
        config_load();
    return &global_config;
}

/* Create example config */
int config_create_example(void) {
    char config_path[4096];
    get_config_path(config_path, sizeof(config_path));

    FILE *f = fopen(config_path, "w");
    if (!f)
        return -1;

    fprintf(f, "# QuickCache Configuration\n\n");
    fprintf(f, "# remote_url=http://quickcache-server:8080\n");
    fprintf(f, "# auth_token=your-secret-token\n");
    fprintf(f, "# timeout=10\n");
    fprintf(f, "# async_upload=true\n");
    fprintf(f, "# ignore_output_path=true\n");

    fclose(f);
    printf("Created example config at: %s\n", config_path);
    return 0;
}
