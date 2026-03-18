#include "ctrl_result_box.h"

#include <string.h>

static uint8_t CtrlResultBox_NextIndex(uint8_t index)
{
    index++;

    if (index >= CTRL_RESULT_BOX_CAPACITY)
        index = 0U;

    return index;
}

void CtrlResult_Clear(CtrlResult *result)
{
    if (result == (void *)0)
        return;

    result->type = CTRL_RESULT_NONE;
    result->hasTargetId = 0U;
    result->targetId = 0U;
    (void)memset(result->text, 0, sizeof(result->text));
}

void CtrlResultBox_Init(CtrlResultBox *box)
{
    uint8_t i;

    if (box == (void *)0)
        return;

    box->head = 0U;
    box->tail = 0U;
    box->count = 0U;

    for (i = 0U; i < CTRL_RESULT_BOX_CAPACITY; i++)
        CtrlResult_Clear(&box->storage[i]);
}

uint8_t CtrlResultBox_HasPending(const CtrlResultBox *box)
{
    if (box == (void *)0)
        return 0U;

    return (box->count != 0U) ? 1U : 0U;
}

uint8_t CtrlResultBox_IsFull(const CtrlResultBox *box)
{
    if (box == (void *)0)
        return 0U;

    return (box->count >= CTRL_RESULT_BOX_CAPACITY) ? 1U : 0U;
}

uint8_t CtrlResultBox_IsEmpty(const CtrlResultBox *box)
{
    if (box == (void *)0)
        return 1U;

    return (box->count == 0U) ? 1U : 0U;
}

uint8_t CtrlResultBox_GetCount(const CtrlResultBox *box)
{
    if (box == (void *)0)
        return 0U;

    return box->count;
}

uint8_t CtrlResultBox_GetCapacity(const CtrlResultBox *box)
{
    (void)box;
    return CTRL_RESULT_BOX_CAPACITY;
}

uint8_t CtrlResultBox_Push(CtrlResultBox *box, const CtrlResult *result)
{
    if (box == (void *)0 || result == (void *)0)
        return 0U;

    if (CtrlResultBox_IsFull(box) != 0U)
        return 0U;

    box->storage[box->tail] = *result;
    box->tail = CtrlResultBox_NextIndex(box->tail);
    box->count++;

    return 1U;
}

uint8_t CtrlResultBox_Pop(CtrlResultBox *box, CtrlResult *outResult)
{
    if (box == (void *)0 || outResult == (void *)0)
        return 0U;

    if (CtrlResultBox_IsEmpty(box) != 0U)
        return 0U;

    *outResult = box->storage[box->head];
    CtrlResult_Clear(&box->storage[box->head]);

    box->head = CtrlResultBox_NextIndex(box->head);
    box->count--;

    return 1U;
}
