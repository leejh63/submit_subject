#include "app_ctrl_result_box.h"

#include <string.h>

static uint8_t AppCtrlResultBox_NextIndex(uint8_t index)
{
    index++;

    if (index >= APP_CTRL_RESULT_BOX_CAPACITY)
        index = 0U;

    return index;
}

void AppCtrlResult_Clear(AppCtrlResult *result)
{
    if (result == (void *)0)
        return;

    result->type = APP_CTRL_RESULT_NONE;
    result->hasTargetId = 0U;
    result->targetId = 0U;
    (void)memset(result->text, 0, sizeof(result->text));
}

void AppCtrlResultBox_Init(AppCtrlResultBox *box)
{
    uint8_t i;

    if (box == (void *)0)
        return;

    box->head = 0U;
    box->tail = 0U;
    box->count = 0U;

    for (i = 0U; i < APP_CTRL_RESULT_BOX_CAPACITY; i++)
        AppCtrlResult_Clear(&box->storage[i]);
}

uint8_t AppCtrlResultBox_HasPending(const AppCtrlResultBox *box)
{
    if (box == (void *)0)
        return 0U;

    return (box->count != 0U) ? 1U : 0U;
}

uint8_t AppCtrlResultBox_IsFull(const AppCtrlResultBox *box)
{
    if (box == (void *)0)
        return 0U;

    return (box->count >= APP_CTRL_RESULT_BOX_CAPACITY) ? 1U : 0U;
}

uint8_t AppCtrlResultBox_IsEmpty(const AppCtrlResultBox *box)
{
    if (box == (void *)0)
        return 1U;

    return (box->count == 0U) ? 1U : 0U;
}

uint8_t AppCtrlResultBox_GetCount(const AppCtrlResultBox *box)
{
    if (box == (void *)0)
        return 0U;

    return box->count;
}

uint8_t AppCtrlResultBox_GetCapacity(const AppCtrlResultBox *box)
{
    if (box == (void *)0)
        return 0U;
    return APP_CTRL_RESULT_BOX_CAPACITY;
}

uint8_t AppCtrlResultBox_Push(AppCtrlResultBox *box, const AppCtrlResult *result)
{
    if (box == (void *)0 || result == (void *)0)
        return 0U;

    if (AppCtrlResultBox_IsFull(box) != 0U)
        return 0U;

    box->storage[box->tail] = *result;
    box->tail = AppCtrlResultBox_NextIndex(box->tail);
    box->count++;

    return 1U;
}

uint8_t AppCtrlResultBox_Pop(AppCtrlResultBox *box, AppCtrlResult *outResult)
{
    if (box == (void *)0 || outResult == (void *)0)
        return 0U;

    if (AppCtrlResultBox_IsEmpty(box) != 0U)
        return 0U;

    *outResult = box->storage[box->head];
    AppCtrlResult_Clear(&box->storage[box->head]);

    box->head = AppCtrlResultBox_NextIndex(box->head);
    box->count--;

    return 1U;
}
