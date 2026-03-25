#include <stddef.h>
#include "infra/emb_fixed_pool.h"

emb_status_t emb_fixed_pool_init(emb_fixed_pool_t *pool,
                                 void *storage,
                                 uint8_t *inUseBitmapBytes,
                                 size_t blockSize,
                                 size_t blockCount)
{
    size_t i;

    if ((pool == 0) || (storage == 0) || (inUseBitmapBytes == 0) ||
        (blockSize == 0U) || (blockCount == 0U))
        return EMB_EINVAL;

    pool->storage = (uint8_t *)storage;
    pool->inUse = inUseBitmapBytes;
    pool->blockSize = blockSize;
    pool->blockCount = blockCount;
    pool->usedCount = 0U;

    for (i = 0U; i < blockCount; ++i)
        pool->inUse[i] = 0U;

    return EMB_OK;
}

void *emb_fixed_pool_alloc(emb_fixed_pool_t *pool)
{
    size_t i;

    if (pool == 0)
        return 0;

    for (i = 0U; i < pool->blockCount; ++i)
    {
        if (pool->inUse[i] == 0U)
        {
            pool->inUse[i] = 1U;
            pool->usedCount++;
            return &pool->storage[i * pool->blockSize];
        }
    }

    return 0;
}

emb_status_t emb_fixed_pool_free(emb_fixed_pool_t *pool, void *ptr)
{
    uintptr_t base;
    uintptr_t addr;
    size_t index;

    if ((pool == 0) || (ptr == 0))
        return EMB_EINVAL;

    base = (uintptr_t)pool->storage;
    addr = (uintptr_t)ptr;

    if ((addr < base) || (addr >= (base + (pool->blockSize * pool->blockCount))))
        return EMB_EINVAL;

    index = (size_t)((addr - base) / pool->blockSize);
    if (pool->inUse[index] == 0U)
        return EMB_ESTATE;

    pool->inUse[index] = 0U;
    pool->usedCount--;
    return EMB_OK;
}

size_t emb_fixed_pool_used(const emb_fixed_pool_t *pool)
{
    return (pool == 0) ? 0U : pool->usedCount;
}
