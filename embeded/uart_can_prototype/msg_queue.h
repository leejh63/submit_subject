#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <stdint.h>
#include "status.h"

/*
 * UART TX용 고정 크기 메시지 큐.
 *
 * 기존에는 슬롯 하나가 512바이트라서 UART UI 문자열 하나를 큐에 넣을 때
 * 메모리 사용량과 memcpy 비용이 너무 컸다.
 * 현재는 "작은 TX chunk" 단위로 큐를 운용한다.
 */

#define MSG_QUEUE_ITEM_DATA_MAX   128U

typedef struct
{
    uint8_t  data[MSG_QUEUE_ITEM_DATA_MAX];
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
