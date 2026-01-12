#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>

int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

int make_dirs(const char *path) {
    char tmp[4096];
    char *p = NULL;
    size_t len;
    
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) == -1 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, 0755) == -1 && errno != EEXIST) {
        return -1;
    }
    
    return 0;
}

int copy_file(const char *src, const char *dst) {
    FILE *fs = fopen(src, "rb");
    if (!fs) return -1;
    
    FILE *fd = fopen(dst, "wb");
    if (!fd) {
        fclose(fs);
        return -1;
    }
    
    unsigned char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fs)) > 0) {
        if (fwrite(buf, 1, n, fd) != n) {
            fclose(fs);
            fclose(fd);
            return -1;
        }
    }
    
    fclose(fs);
    fclose(fd);
    return 0;
}

char* read_file(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buf = malloc(fsize + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    
    size_t n = fread(buf, 1, fsize, f);
    buf[n] = 0;
    
    if (len) *len = n;
    fclose(f);
    return buf;
}

int write_file(const char *path, const void *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    
    size_t written = fwrite(data, 1, len, f);
    fclose(f);
    
    return (written == len) ? 0 : -1;
}

void get_home_dir(char *buf, size_t len) {
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    snprintf(buf, len, "%s", home);
}
