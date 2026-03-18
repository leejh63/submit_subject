#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <stdint.h>
#include "status.h"

#define MSG_QUEUE_ITEM_DATA_MAX   512U

typedef struct
{
    uint8_t data[MSG_QUEUE_ITEM_DATA_MAX];
    uint16_t length;
} MsgQueueItem;

typedef struct
{
    MsgQueueItem *storage;
    uint16_t capacity;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} MsgQueue;

status_t MsgQueue_Init(MsgQueue *queue,
                       MsgQueueItem *storage,
                       uint16_t capacity);

void MsgQueue_Reset(MsgQueue *queue);

status_t MsgQueue_Push(MsgQueue *queue,
                       const uint8_t *data,
                       uint16_t length);

status_t MsgQueue_Pop(MsgQueue *queue,
                      MsgQueueItem *item);

uint8_t MsgQueue_IsEmpty(const MsgQueue *queue);
uint8_t MsgQueue_IsFull(const MsgQueue *queue);

uint16_t MsgQueue_GetCount(const MsgQueue *queue);
uint16_t MsgQueue_GetCapacity(const MsgQueue *queue);

#endif
