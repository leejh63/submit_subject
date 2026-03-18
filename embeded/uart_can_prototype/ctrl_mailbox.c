#include "ctrl_mailbox.h"

static uint8_t CtrlMailbox_NextIndex(uint8_t index)
{
    index++;

    if (index >= CTRL_MAILBOX_CAPACITY)
        index = 0U;

    return index;
}

void CtrlMailbox_Init(CtrlMailbox *mailbox)
{
    uint8_t i;

    if (mailbox == (void *)0)
        return;

    mailbox->head = 0U;
    mailbox->tail = 0U;
    mailbox->count = 0U;

    for (i = 0U; i < CTRL_MAILBOX_CAPACITY; i++)
        CtrlCmd_Clear(&mailbox->storage[i]);
}

uint8_t CtrlMailbox_HasPending(const CtrlMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 0U;

    return (mailbox->count != 0U) ? 1U : 0U;
}

uint8_t CtrlMailbox_IsFull(const CtrlMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 0U;

    return (mailbox->count >= CTRL_MAILBOX_CAPACITY) ? 1U : 0U;
}

uint8_t CtrlMailbox_IsEmpty(const CtrlMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 1U;

    return (mailbox->count == 0U) ? 1U : 0U;
}

uint8_t CtrlMailbox_GetCount(const CtrlMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 0U;

    return mailbox->count;
}

uint8_t CtrlMailbox_GetCapacity(const CtrlMailbox *mailbox)
{
    (void)mailbox;
    return CTRL_MAILBOX_CAPACITY;
}

uint8_t CtrlMailbox_Push(CtrlMailbox *mailbox, const CtrlCmd *cmd)
{
    if (mailbox == (void *)0 || cmd == (void *)0)
        return 0U;

    if (CtrlMailbox_IsFull(mailbox) != 0U)
        return 0U;

    mailbox->storage[mailbox->tail] = *cmd;
    mailbox->tail = CtrlMailbox_NextIndex(mailbox->tail);
    mailbox->count++;

    return 1U;
}

uint8_t CtrlMailbox_Pop(CtrlMailbox *mailbox, CtrlCmd *outCmd)
{
    if (mailbox == (void *)0 || outCmd == (void *)0)
        return 0U;

    if (CtrlMailbox_IsEmpty(mailbox) != 0U)
        return 0U;

    *outCmd = mailbox->storage[mailbox->head];
    CtrlCmd_Clear(&mailbox->storage[mailbox->head]);

    mailbox->head = CtrlMailbox_NextIndex(mailbox->head);
    mailbox->count--;

    return 1U;
}
