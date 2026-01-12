#include "compress.h"
#include <stdio.h>
#include <stdlib.h>
#include <zstd.h>

#define CHUNK_SIZE (128 * 1024)

int compress_file(const char *src, const char *dst, size_t *compressed_size) {
    FILE *fin = fopen(src, "rb");
    if (!fin) return -1;

    FILE *fout = fopen(dst, "wb");
    if (!fout) {
        fclose(fin);
        return -1;
    }

    ZSTD_CCtx *cctx = ZSTD_createCCtx();
    if (!cctx) {
        fclose(fin);
        fclose(fout);
        return -1;
    }

    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, 3);

    size_t total_compressed = 0;
    unsigned char in_buf[CHUNK_SIZE];
    unsigned char out_buf[ZSTD_compressBound(CHUNK_SIZE)];

    size_t n;
    while ((n = fread(in_buf, 1, sizeof(in_buf), fin)) > 0) {
        size_t compressed = ZSTD_compressCCtx(cctx, out_buf, sizeof(out_buf), in_buf, n, 3);
        
        if (ZSTD_isError(compressed)) {
            ZSTD_freeCCtx(cctx);
            fclose(fin);
            fclose(fout);
            return -1;
        }

        if (fwrite(out_buf, 1, compressed, fout) != compressed) {
            ZSTD_freeCCtx(cctx);
            fclose(fin);
            fclose(fout);
            return -1;
        }

        total_compressed += compressed;
    }

    ZSTD_freeCCtx(cctx);
    fclose(fin);
    fclose(fout);

    if (compressed_size) {
        *compressed_size = total_compressed;
    }

    return 0;
}

int decompress_file(const char *src, const char *dst) {
    FILE *fin = fopen(src, "rb");
    if (!fin) return -1;

    FILE *fout = fopen(dst, "wb");
    if (!fout) {
        fclose(fin);
        return -1;
    }

    ZSTD_DCtx *dctx = ZSTD_createDCtx();
    if (!dctx) {
        fclose(fin);
        fclose(fout);
        return -1;
    }

    unsigned char in_buf[CHUNK_SIZE];
    unsigned char out_buf[CHUNK_SIZE];

    size_t n;
    while ((n = fread(in_buf, 1, sizeof(in_buf), fin)) > 0) {
        size_t decompressed = ZSTD_decompressDCtx(dctx, out_buf, sizeof(out_buf), in_buf, n);
        
        if (ZSTD_isError(decompressed)) {
            ZSTD_freeDCtx(dctx);
            fclose(fin);
            fclose(fout);
            return -1;
        }

        if (fwrite(out_buf, 1, decompressed, fout) != decompressed) {
            ZSTD_freeDCtx(dctx);
            fclose(fin);
            fclose(fout);
            return -1;
        }
    }

    ZSTD_freeDCtx(dctx);
    fclose(fin);
    fclose(fout);

    return 0;
}
