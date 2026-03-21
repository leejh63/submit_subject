#include "infra/emb_ring_u8.h"

static size_t emb_ring_u8_next(const emb_ring_u8_t *rb, size_t index)
{
    return (index + 1U) % rb->capacity;
}

emb_status_t emb_ring_u8_init(emb_ring_u8_t *rb, uint8_t *buffer, size_t capacity)
{
    if ((rb == 0) || (buffer == 0) || (capacity == 0U))
        return EMB_EINVAL;

    rb->buffer = buffer;
    rb->capacity = capacity;
    rb->head = 0U;
    rb->tail = 0U;
    rb->count = 0U;
    return EMB_OK;
}

emb_status_t emb_ring_u8_push(emb_ring_u8_t *rb, uint8_t value)
{
    if (rb == 0)
        return EMB_EINVAL;
    if (rb->count >= rb->capacity)
        return EMB_ENOSPACE;

    rb->buffer[rb->head] = value;
    rb->head = emb_ring_u8_next(rb, rb->head);
    rb->count++;
    return EMB_OK;
}

emb_status_t emb_ring_u8_pop(emb_ring_u8_t *rb, uint8_t *out)
{
    if ((rb == 0) || (out == 0))
        return EMB_EINVAL;
    if (rb->count == 0U)
        return EMB_ENOENT;

    *out = rb->buffer[rb->tail];
    rb->tail = emb_ring_u8_next(rb, rb->tail);
    rb->count--;
    return EMB_OK;
}

size_t emb_ring_u8_count(const emb_ring_u8_t *rb)
{
    return (rb == 0) ? 0U : rb->count;
}

size_t emb_ring_u8_space(const emb_ring_u8_t *rb)
{
    return (rb == 0) ? 0U : (rb->capacity - rb->count);
}

void emb_ring_u8_clear(emb_ring_u8_t *rb)
{
    if (rb == 0)
        return;

    rb->head = 0U;
    rb->tail = 0U;
    rb->count = 0U;
}
