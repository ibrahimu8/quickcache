#include "hash.h"
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

int hash_file(const char *filename, hash_t out) {
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        fclose(f);
        return -1;
    }
    
    if (!EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)) {
        EVP_MD_CTX_free(ctx);
        fclose(f);
        return -1;
    }
    
    unsigned char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        if (!EVP_DigestUpdate(ctx, buf, n)) {
            EVP_MD_CTX_free(ctx);
            fclose(f);
            return -1;
        }
    }
    
    unsigned int len;
    if (!EVP_DigestFinal_ex(ctx, out, &len) || len != HASH_SIZE) {
        EVP_MD_CTX_free(ctx);
        fclose(f);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    fclose(f);
    return 0;
}

int hash_data(const void *data, size_t len, hash_t out) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;
    
    if (!EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    if (!EVP_DigestUpdate(ctx, data, len)) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    unsigned int out_len;
    if (!EVP_DigestFinal_ex(ctx, out, &out_len) || out_len != HASH_SIZE) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    return 0;
}

int hash_combine(const hash_t h1, const hash_t h2, hash_t out) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;
    
    if (!EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    if (!EVP_DigestUpdate(ctx, h1, HASH_SIZE) ||
        !EVP_DigestUpdate(ctx, h2, HASH_SIZE)) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    unsigned int out_len;
    if (!EVP_DigestFinal_ex(ctx, out, &out_len) || out_len != HASH_SIZE) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    return 0;
}

void hash_to_hex(const hash_t hash, char *hex) {
    for (int i = 0; i < HASH_SIZE; i++) {
        sprintf(hex + (i * 2), "%02x", hash[i]);
    }
    hex[HASH_HEX_SIZE - 1] = '\0';
}
