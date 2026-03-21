#include <string.h>
#include "infra/emb_msg_queue.h"

static uint8_t *emb_msg_queue_slot(const emb_msg_queue_t *q, size_t index)
{
    return &q->storage[index * q->elementSize];
}

emb_status_t emb_msg_queue_init(emb_msg_queue_t *q,
                                void *storage,
                                size_t elementSize,
                                size_t capacity)
{
    if ((q == 0) || (storage == 0) || (elementSize == 0U) || (capacity == 0U))
        return EMB_EINVAL;

    q->storage = (uint8_t *)storage;
    q->elementSize = elementSize;
    q->capacity = capacity;
    q->head = 0U;
    q->tail = 0U;
    q->count = 0U;
    return EMB_OK;
}

emb_status_t emb_msg_queue_push(emb_msg_queue_t *q, const void *element)
{
    if ((q == 0) || (element == 0))
        return EMB_EINVAL;
    if (q->count >= q->capacity)
        return EMB_ENOSPACE;

    memcpy(emb_msg_queue_slot(q, q->head), element, q->elementSize);
    q->head = (q->head + 1U) % q->capacity;
    q->count++;
    return EMB_OK;
}

emb_status_t emb_msg_queue_pop(emb_msg_queue_t *q, void *outElement)
{
    if ((q == 0) || (outElement == 0))
        return EMB_EINVAL;
    if (q->count == 0U)
        return EMB_ENOENT;

    memcpy(outElement, emb_msg_queue_slot(q, q->tail), q->elementSize);
    q->tail = (q->tail + 1U) % q->capacity;
    q->count--;
    return EMB_OK;
}

size_t emb_msg_queue_count(const emb_msg_queue_t *q)
{
    return (q == 0) ? 0U : q->count;
}

void emb_msg_queue_clear(emb_msg_queue_t *q)
{
    if (q == 0)
        return;

    q->head = 0U;
    q->tail = 0U;
    q->count = 0U;
}
