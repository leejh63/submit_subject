/*
 * 고정 크기 ring queue 구현부다.
 * queue는 caller가 제공한 저장 공간에 item 전체를 복사하며,
 * 예측 가능한 단일 스레드 모듈 버퍼링을 목표로 한다.
 */
#include "infra_queue.h"

#include <stddef.h>
#include <string.h>

/*
 * wrap-around를 고려해 ring index를 한 칸 전진시킨다.
 * queue 포인터 계산을 한곳에 모아두면,
 * push, pop, reset 로직을 작고 일관되게 유지할 수 있다.
 */
static uint16_t InfraQueue_NextIndex(const InfraQueue *queue, uint16_t index)
{
    index++;
    if (index >= queue->capacity)
    {
        index = 0U;
    }

    return index;
}

/*
 * queue 메타데이터를 caller가 제공한 저장 공간에 연결한다.
 * 버퍼를 먼저 비워서,
 * 모듈이 오래된 데이터 없이 예측 가능한 빈 queue 상태에서 시작하게 한다.
 */
InfraStatus InfraQueue_Init(InfraQueue *queue,
                            void *storage,
                            uint16_t item_size,
                            uint16_t capacity)
{
    if ((queue == NULL) || (storage == NULL) || (item_size == 0U) || (capacity == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    queue->buffer = (uint8_t *)storage;
    queue->item_size = item_size;
    queue->capacity = capacity;
    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;

    (void)memset(queue->buffer, 0, (size_t)item_size * (size_t)capacity);

    return INFRA_STATUS_OK;
}

void InfraQueue_Reset(InfraQueue *queue)
{
    if (queue == NULL)
    {
        return;
    }

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;

    if ((queue->buffer != NULL) && (queue->item_size != 0U) && (queue->capacity != 0U))
    {
        (void)memset(queue->buffer,
                     0,
                     (size_t)queue->item_size * (size_t)queue->capacity);
    }
}

/*
 * ring queue의 tail 슬롯에 item 하나를 복사한다.
 * queue가 가득 찬 경우를 명시적으로 알려서,
 * producer가 재시도, drop, backpressure 전달 여부를 결정할 수 있게 한다.
 */
InfraStatus InfraQueue_Push(InfraQueue *queue, const void *item)
{
    uint8_t *slot;

    if ((queue == NULL) || (item == NULL) || (queue->buffer == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (queue->count >= queue->capacity)
    {
        return INFRA_STATUS_FULL;
    }

    slot = &queue->buffer[(size_t)queue->tail * (size_t)queue->item_size];
    (void)memcpy(slot, item, queue->item_size);

    queue->tail = InfraQueue_NextIndex(queue, queue->tail);
    queue->count++;

    return INFRA_STATUS_OK;
}

/*
 * ring queue의 head에서 item 하나를 꺼낸다.
 * 꺼낸 슬롯을 0으로 지워서,
 * 디버깅을 쉽게 하고 오래된 payload가 남지 않게 한다.
 */
InfraStatus InfraQueue_Pop(InfraQueue *queue, void *out_item)
{
    uint8_t *slot;

    if ((queue == NULL) || (out_item == NULL) || (queue->buffer == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (queue->count == 0U)
    {
        return INFRA_STATUS_EMPTY;
    }

    slot = &queue->buffer[(size_t)queue->head * (size_t)queue->item_size];
    (void)memcpy(out_item, slot, queue->item_size);
    (void)memset(slot, 0, queue->item_size);

    queue->head = InfraQueue_NextIndex(queue, queue->head);
    queue->count--;

    return INFRA_STATUS_OK;
}

InfraStatus InfraQueue_Peek(const InfraQueue *queue, void *out_item)
{
    const uint8_t *slot;

    if ((queue == NULL) || (out_item == NULL) || (queue->buffer == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (queue->count == 0U)
    {
        return INFRA_STATUS_EMPTY;
    }

    slot = &queue->buffer[(size_t)queue->head * (size_t)queue->item_size];
    (void)memcpy(out_item, slot, queue->item_size);

    return INFRA_STATUS_OK;
}

uint16_t InfraQueue_GetCount(const InfraQueue *queue)
{
    if (queue == NULL)
    {
        return 0U;
    }

    return queue->count;
}

uint16_t InfraQueue_GetCapacity(const InfraQueue *queue)
{
    if (queue == NULL)
    {
        return 0U;
    }

    return queue->capacity;
}

uint8_t InfraQueue_IsEmpty(const InfraQueue *queue)
{
    if (queue == NULL)
    {
        return 1U;
    }

    return (queue->count == 0U) ? 1U : 0U;
}

uint8_t InfraQueue_IsFull(const InfraQueue *queue)
{
    if (queue == NULL)
    {
        return 1U;
    }

    return (queue->count >= queue->capacity) ? 1U : 0U;
}
