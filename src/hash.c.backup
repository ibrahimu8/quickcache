#include "hash.h"
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

int hash_file(const char *path, hash_t out) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    
    unsigned char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        SHA256_Update(&ctx, buf, n);
    }
    
    SHA256_Final(out, &ctx);
    fclose(f);
    return 0;
}

void hash_data(const void *data, size_t len, hash_t out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}

void hash_to_hex(const hash_t hash, char *hex) {
    for (int i = 0; i < HASH_SIZE; i++) {
        sprintf(hex + (i * 2), "%02x", hash[i]);
    }
    hex[HASH_HEX_SIZE - 1] = '\0';
}

void hash_combine(const hash_t h1, const hash_t h2, hash_t out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, h1, HASH_SIZE);
    SHA256_Update(&ctx, h2, HASH_SIZE);
    SHA256_Final(out, &ctx);
}
