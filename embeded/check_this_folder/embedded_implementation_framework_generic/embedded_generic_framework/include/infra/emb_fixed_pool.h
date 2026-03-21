#ifndef EMB_FIXED_POOL_H
#define EMB_FIXED_POOL_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct
{
    uint8_t *storage;
    uint8_t *inUse;
    size_t blockSize;
    size_t blockCount;
    size_t usedCount;
} emb_fixed_pool_t;

emb_status_t emb_fixed_pool_init(emb_fixed_pool_t *pool,
                                 void *storage,
                                 uint8_t *inUseBitmapBytes,
                                 size_t blockSize,
                                 size_t blockCount);
void *emb_fixed_pool_alloc(emb_fixed_pool_t *pool);
emb_status_t emb_fixed_pool_free(emb_fixed_pool_t *pool, void *ptr);
size_t emb_fixed_pool_used(const emb_fixed_pool_t *pool);

#endif
