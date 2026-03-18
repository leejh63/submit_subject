#ifndef CTRL_MAILBOX_H
#define CTRL_MAILBOX_H

#include <stdint.h>

#include "ctrl_cmd.h"

#define CTRL_MAILBOX_CAPACITY 4U

typedef struct
{
    CtrlCmd storage[CTRL_MAILBOX_CAPACITY];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} CtrlMailbox;

void CtrlMailbox_Init(CtrlMailbox *mailbox);

uint8_t CtrlMailbox_HasPending(const CtrlMailbox *mailbox);

uint8_t CtrlMailbox_IsFull(const CtrlMailbox *mailbox);

uint8_t CtrlMailbox_IsEmpty(const CtrlMailbox *mailbox);

uint8_t CtrlMailbox_GetCount(const CtrlMailbox *mailbox);

uint8_t CtrlMailbox_GetCapacity(const CtrlMailbox *mailbox);

uint8_t CtrlMailbox_Push(CtrlMailbox *mailbox, const CtrlCmd *cmd);

uint8_t CtrlMailbox_Pop(CtrlMailbox *mailbox, CtrlCmd *outCmd);

#endif
