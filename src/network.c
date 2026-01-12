#define _POSIX_C_SOURCE 200809L  // Must be at the very top before any headers

#include <unistd.h>  // For usleep()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include "network.h"
#include "config.h"
#include "hash.h"
#include "utils.h"

typedef struct {
    hash_t key;
    char file_path[4096];
} upload_job_t;

static pthread_mutex_t upload_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static upload_job_t *upload_queue = NULL;
static int upload_queue_size = 0;
static int upload_queue_capacity = 0;
static pthread_t upload_thread;
static int upload_thread_running = 0;

// Callback for writing downloaded data
static size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// Callback for reading upload data
static size_t read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fread(ptr, size, nmemb, stream);
}

int network_init(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return 0;
}

void network_cleanup(void) {
    if (upload_thread_running) {
        upload_thread_running = 0;
        pthread_join(upload_thread, NULL);
    }
    curl_global_cleanup();
}

int network_get(const hash_t key, const char *output_path) {
    const quickcache_config_t *cfg = config_get();
    if (!cfg->remote_enabled) return -1;

    char hex[HASH_HEX_SIZE];
    hash_to_hex(key, hex);

    char url[1024];
    snprintf(url, sizeof(url), "%s/cache/%s", cfg->remote_url, hex);

    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    FILE *f = fopen(output_path, "wb");
    if (!f) {
        curl_easy_cleanup(curl);
        return -1;
    }

    struct curl_slist *headers = NULL;
    if (cfg->auth_token[0] != '\0') {
        char auth_header[512];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", cfg->auth_token);
        headers = curl_slist_append(headers, auth_header);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, cfg->timeout_seconds);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    fclose(f);

    if (res != CURLE_OK || http_code != 200) {
        unlink(output_path);
        return -1;
    }

    return 0;
}

int network_put(const hash_t key, const char *file_path) {
    const quickcache_config_t *cfg = config_get();
    if (!cfg->remote_enabled) return -1;

    char hex[HASH_HEX_SIZE];
    hash_to_hex(key, hex);

    char url[1024];
    snprintf(url, sizeof(url), "%s/cache/%s", cfg->remote_url, hex);

    FILE *f = fopen(file_path, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    CURL *curl = curl_easy_init();
    if (!curl) {
        fclose(f);
        return -1;
    }

    struct curl_slist *headers = NULL;
    if (cfg->auth_token[0] != '\0') {
        char auth_header[512];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", cfg->auth_token);
        headers = curl_slist_append(headers, auth_header);
    }
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, f);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_size);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, cfg->timeout_seconds * 2);
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    fclose(f);

    if (res != CURLE_OK || (http_code != 200 && http_code != 201)) {
        return -1;
    }

    return 0;
}

// Background upload thread
static void* upload_worker(void *arg) {
    (void)arg;

    while (upload_thread_running) {
        upload_job_t job;
        int has_job = 0;

        pthread_mutex_lock(&upload_queue_mutex);
        if (upload_queue_size > 0) {
            job = upload_queue[0];
            for (int i = 1; i < upload_queue_size; i++) {
                upload_queue[i - 1] = upload_queue[i];
            }
            upload_queue_size--;
            has_job = 1;
        }
        pthread_mutex_unlock(&upload_queue_mutex);

        if (has_job) {
            network_put(job.key, job.file_path);
        } else {
            // Fixed usleep issue
            struct timespec ts = {0, 100000000}; // 100ms
            nanosleep(&ts, NULL);
        }
    }

    return NULL;
}

void network_put_async(const hash_t key, const char *file_path) {
    const quickcache_config_t *cfg = config_get();
    if (!cfg->remote_enabled || !cfg->async_upload) {
        network_put(key, file_path);
        return;
    }

    if (!upload_thread_running) {
        upload_thread_running = 1;
        pthread_create(&upload_thread, NULL, upload_worker, NULL);
    }

    pthread_mutex_lock(&upload_queue_mutex);

    if (upload_queue_size >= upload_queue_capacity) {
        upload_queue_capacity = upload_queue_capacity == 0 ? 16 : upload_queue_capacity * 2;
        upload_queue = realloc(upload_queue, upload_queue_capacity * sizeof(upload_job_t));
    }

    memcpy(upload_queue[upload_queue_size].key, key, HASH_SIZE);
    strncpy(upload_queue[upload_queue_size].file_path, file_path,
            sizeof(upload_queue[upload_queue_size].file_path) - 1);
    upload_queue_size++;

    pthread_mutex_unlock(&upload_queue_mutex);
}

int network_check_exists(const hash_t key) {
    const quickcache_config_t *cfg = config_get();
    if (!cfg->remote_enabled) return 0;

    char hex[HASH_HEX_SIZE];
    hash_to_hex(key, hex);

    char url[1024];
    snprintf(url, sizeof(url), "%s/cache/%s", cfg->remote_url, hex);

    CURL *curl = curl_easy_init();
    if (!curl) return 0;

    struct curl_slist *headers = NULL;
    if (cfg->auth_token[0] != '\0') {
        char auth_header[512];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", cfg->auth_token);
        headers = curl_slist_append(headers, auth_header);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, cfg->timeout_seconds);
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && http_code == 200) ? 1 : 0;
}
