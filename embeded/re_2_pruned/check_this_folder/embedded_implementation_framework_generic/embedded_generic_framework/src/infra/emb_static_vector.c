#include <string.h>
#include "infra/emb_static_vector.h"

static uint8_t *emb_static_vector_slot(const emb_static_vector_t *vec, size_t index)
{
    return &vec->storage[index * vec->elementSize];
}

emb_status_t emb_static_vector_init(emb_static_vector_t *vec,
                                    void *storage,
                                    size_t elementSize,
                                    size_t capacity)
{
    if ((vec == 0) || (storage == 0) || (elementSize == 0U) || (capacity == 0U))
        return EMB_EINVAL;

    vec->storage = (uint8_t *)storage;
    vec->elementSize = elementSize;
    vec->capacity = capacity;
    vec->count = 0U;
    return EMB_OK;
}

emb_status_t emb_static_vector_push_back(emb_static_vector_t *vec, const void *element)
{
    if ((vec == 0) || (element == 0))
        return EMB_EINVAL;
    if (vec->count >= vec->capacity)
        return EMB_ENOSPACE;

    memcpy(emb_static_vector_slot(vec, vec->count), element, vec->elementSize);
    vec->count++;
    return EMB_OK;
}

emb_status_t emb_static_vector_get(const emb_static_vector_t *vec, size_t index, void *outElement)
{
    if ((vec == 0) || (outElement == 0))
        return EMB_EINVAL;
    if (index >= vec->count)
        return EMB_ENOENT;

    memcpy(outElement, emb_static_vector_slot(vec, index), vec->elementSize);
    return EMB_OK;
}

emb_status_t emb_static_vector_remove_at(emb_static_vector_t *vec, size_t index)
{
    uint8_t *dst;
    uint8_t *src;
    size_t bytes;

    if (vec == 0)
        return EMB_EINVAL;
    if (index >= vec->count)
        return EMB_ENOENT;

    if (index < (vec->count - 1U))
    {
        dst = emb_static_vector_slot(vec, index);
        src = emb_static_vector_slot(vec, index + 1U);
        bytes = (vec->count - index - 1U) * vec->elementSize;
        memmove(dst, src, bytes);
    }

    vec->count--;
    return EMB_OK;
}
