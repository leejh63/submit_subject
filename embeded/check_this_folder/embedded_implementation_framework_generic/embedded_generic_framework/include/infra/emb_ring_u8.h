#ifndef EMB_RING_U8_H
#define EMB_RING_U8_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct
{
    uint8_t *buffer;
    size_t capacity;
    volatile size_t head;
    volatile size_t tail;
    volatile size_t count;
} emb_ring_u8_t;

emb_status_t emb_ring_u8_init(emb_ring_u8_t *rb, uint8_t *buffer, size_t capacity);
emb_status_t emb_ring_u8_push(emb_ring_u8_t *rb, uint8_t value);
emb_status_t emb_ring_u8_pop(emb_ring_u8_t *rb, uint8_t *out);
size_t emb_ring_u8_count(const emb_ring_u8_t *rb);
size_t emb_ring_u8_space(const emb_ring_u8_t *rb);
void emb_ring_u8_clear(emb_ring_u8_t *rb);

#endif
