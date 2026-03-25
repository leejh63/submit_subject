#ifndef EMB_BITMAP_H
#define EMB_BITMAP_H

#include <stddef.h>
#include <stdint.h>

void emb_bitmap_set(uint32_t *bitmapWords, size_t bitIndex);
void emb_bitmap_clear(uint32_t *bitmapWords, size_t bitIndex);
uint8_t emb_bitmap_test(const uint32_t *bitmapWords, size_t bitIndex);

#endif
