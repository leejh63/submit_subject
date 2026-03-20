#include "msg_queue.h"

#include <stddef.h>
#include <string.h>

status_t MsgQueue_Init(MsgQueue *queue,
                       MsgQueueItem *storage,
                       uint16_t capacity)
{
    if ((queue == (void *)0) || (storage == (void *)0) || (capacity == 0U))
        return STATUS_ERROR;

    queue->storage = storage;
    queue->capacity = capacity;
    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;

    return STATUS_SUCCESS;
}

void MsgQueue_Reset(MsgQueue *queue)
{
    if (queue == (void *)0)
        return;

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;
}

uint8_t MsgQueue_IsEmpty(const MsgQueue *queue)
{
    if (queue == (void *)0)
        return 1U;

    return (queue->count == 0U) ? 1U : 0U;
}

uint8_t MsgQueue_IsFull(const MsgQueue *queue)
{
    if (queue == (void *)0)
        return 1U;

    return (queue->count >= queue->capacity) ? 1U : 0U;
}

uint16_t MsgQueue_GetCount(const MsgQueue *queue)
{
    if (queue == (void *)0)
        return 0U;

    return queue->count;
}

uint16_t MsgQueue_GetCapacity(const MsgQueue *queue)
{
    if (queue == (void *)0)
        return 0U;

    return queue->capacity;
}

status_t MsgQueue_Push(MsgQueue *queue,
                       const uint8_t *data,
                       uint16_t length)
{
    MsgQueueItem *slot;

    if ((queue == (void *)0) || (data == (void *)0))
        return STATUS_ERROR;

    if ((length == 0U) || (length > MSG_QUEUE_ITEM_DATA_MAX))
        return STATUS_ERROR;

    if (MsgQueue_IsFull(queue) != 0U)
        return STATUS_BUSY;

    slot = &queue->storage[queue->tail];
    (void)memcpy(slot->data, data, length);
    slot->length = length;

    queue->tail++;
    if (queue->tail >= queue->capacity)
        queue->tail = 0U;

    queue->count++;
    return STATUS_SUCCESS;
}

status_t MsgQueue_Pop(MsgQueue *queue,
                      MsgQueueItem *outItem)
{
    MsgQueueItem *slot;

    if ((queue == (void *)0) || (outItem == (void *)0))
        return STATUS_ERROR;

    if (MsgQueue_IsEmpty(queue) != 0U)
        return STATUS_BUSY;

    slot = &queue->storage[queue->head];
    (void)memcpy(outItem, slot, sizeof(MsgQueueItem));
    slot->length = 0U;

    queue->head++;
    if (queue->head >= queue->capacity)
        queue->head = 0U;

    queue->count--;
    return STATUS_SUCCESS;
}
