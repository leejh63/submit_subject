#ifndef CTRL_RESULT_BOX_H
#define CTRL_RESULT_BOX_H

#include <stdint.h>

#define CTRL_RESULT_TEXT_SIZE 64U
#define CTRL_RESULT_BOX_CAPACITY 4U

typedef enum
{
    CTRL_RESULT_NONE = 0,
    CTRL_RESULT_OK,
    CTRL_RESULT_ERROR,
    CTRL_RESULT_TIMEOUT,
    CTRL_RESULT_DENIED,
    CTRL_RESULT_INVALID_TARGET
} CtrlResultType;

typedef struct
{
    CtrlResultType type;

    uint8_t        hasTargetId;
    uint8_t        targetId;

    char           text[CTRL_RESULT_TEXT_SIZE];
} CtrlResult;

typedef struct
{
    CtrlResult storage[CTRL_RESULT_BOX_CAPACITY];
    uint8_t    head;
    uint8_t    tail;
    uint8_t    count;
} CtrlResultBox;

void CtrlResult_Clear(CtrlResult *result);

void CtrlResultBox_Init(CtrlResultBox *box);
uint8_t CtrlResultBox_HasPending(const CtrlResultBox *box);

uint8_t CtrlResultBox_IsFull(const CtrlResultBox *box);
uint8_t CtrlResultBox_IsEmpty(const CtrlResultBox *box);

uint8_t CtrlResultBox_GetCount(const CtrlResultBox *box);
uint8_t CtrlResultBox_GetCapacity(const CtrlResultBox *box);

uint8_t CtrlResultBox_Push(CtrlResultBox *box, const CtrlResult *result);
uint8_t CtrlResultBox_Pop(CtrlResultBox *box, CtrlResult *outResult);

#endif
