#include "infra/emb_bitmap.h"

void emb_bitmap_set(uint32_t *bitmapWords, size_t bitIndex)
{
    bitmapWords[bitIndex / 32U] |= (uint32_t)(1UL << (bitIndex % 32U));
}

void emb_bitmap_clear(uint32_t *bitmapWords, size_t bitIndex)
{
    bitmapWords[bitIndex / 32U] &= (uint32_t)~(1UL << (bitIndex % 32U));
}

uint8_t emb_bitmap_test(const uint32_t *bitmapWords, size_t bitIndex)
{
    return (bitmapWords[bitIndex / 32U] & (uint32_t)(1UL << (bitIndex % 32U))) ? 1U : 0U;
}
