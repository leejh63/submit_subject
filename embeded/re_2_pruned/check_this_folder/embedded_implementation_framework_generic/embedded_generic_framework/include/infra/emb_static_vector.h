#ifndef EMB_STATIC_VECTOR_H
#define EMB_STATIC_VECTOR_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct
{
    uint8_t *storage;
    size_t elementSize;
    size_t capacity;
    size_t count;
} emb_static_vector_t;

emb_status_t emb_static_vector_init(emb_static_vector_t *vec,
                                    void *storage,
                                    size_t elementSize,
                                    size_t capacity);
emb_status_t emb_static_vector_push_back(emb_static_vector_t *vec, const void *element);
emb_status_t emb_static_vector_get(const emb_static_vector_t *vec, size_t index, void *outElement);
emb_status_t emb_static_vector_remove_at(emb_static_vector_t *vec, size_t index);

#endif
