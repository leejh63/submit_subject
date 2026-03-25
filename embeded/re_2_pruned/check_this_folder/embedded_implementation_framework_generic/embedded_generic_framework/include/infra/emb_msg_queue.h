#ifndef EMB_MSG_QUEUE_H
#define EMB_MSG_QUEUE_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct
{
    uint8_t *storage;
    size_t elementSize;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
} emb_msg_queue_t;

emb_status_t emb_msg_queue_init(emb_msg_queue_t *q,
                                void *storage,
                                size_t elementSize,
                                size_t capacity);
emb_status_t emb_msg_queue_push(emb_msg_queue_t *q, const void *element);
emb_status_t emb_msg_queue_pop(emb_msg_queue_t *q, void *outElement);
size_t emb_msg_queue_count(const emb_msg_queue_t *q);
void emb_msg_queue_clear(emb_msg_queue_t *q);

#endif
