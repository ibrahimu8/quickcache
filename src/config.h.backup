#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int remote_enabled;
    char remote_url[512];
    char auth_token[256];
    int timeout_seconds;
    int async_upload;
} quickcache_config_t;

int config_load(void);
const quickcache_config_t* config_get(void);
int config_create_example(void);

#endif
