#include "app_ctrl_command_mailbox.h"

static uint8_t AppCtrlCommandMailbox_NextIndex(uint8_t index)
{
    index++;

    if (index >= APP_CTRL_COMMAND_MAILBOX_CAPACITY)
        index = 0U;

    return index;
}

void AppCtrlCommandMailbox_Init(AppCtrlCommandMailbox *mailbox)
{
    uint8_t i;

    if (mailbox == (void *)0)
        return;

    mailbox->head = 0U;
    mailbox->tail = 0U;
    mailbox->count = 0U;

    for (i = 0U; i < APP_CTRL_COMMAND_MAILBOX_CAPACITY; i++)
        AppCtrlCommand_Clear(&mailbox->storage[i]);
}

uint8_t AppCtrlCommandMailbox_HasPending(const AppCtrlCommandMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 0U;

    return (mailbox->count != 0U) ? 1U : 0U;
}

uint8_t AppCtrlCommandMailbox_IsFull(const AppCtrlCommandMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 0U;

    return (mailbox->count >= APP_CTRL_COMMAND_MAILBOX_CAPACITY) ? 1U : 0U;
}

uint8_t AppCtrlCommandMailbox_IsEmpty(const AppCtrlCommandMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 1U;

    return (mailbox->count == 0U) ? 1U : 0U;
}

uint8_t AppCtrlCommandMailbox_GetCount(const AppCtrlCommandMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 0U;

    return mailbox->count;
}
// 수정함
uint8_t AppCtrlCommandMailbox_GetCapacity(const AppCtrlCommandMailbox *mailbox)
{
    if (mailbox == (void *)0)
        return 0U;
    return APP_CTRL_COMMAND_MAILBOX_CAPACITY;
}

uint8_t AppCtrlCommandMailbox_Push(AppCtrlCommandMailbox *mailbox, const AppCtrlCommand *cmd)
{
    if (mailbox == (void *)0 || cmd == (void *)0)
        return 0U;

    if (AppCtrlCommandMailbox_IsFull(mailbox) != 0U)
        return 0U;

    mailbox->storage[mailbox->tail] = *cmd;
    mailbox->tail = AppCtrlCommandMailbox_NextIndex(mailbox->tail);
    mailbox->count++;

    return 1U;
}

uint8_t AppCtrlCommandMailbox_Pop(AppCtrlCommandMailbox *mailbox, AppCtrlCommand *outCmd)
{
    if (mailbox == (void *)0 || outCmd == (void *)0)
        return 0U;

    if (AppCtrlCommandMailbox_IsEmpty(mailbox) != 0U)
        return 0U;

    *outCmd = mailbox->storage[mailbox->head];
    AppCtrlCommand_Clear(&mailbox->storage[mailbox->head]);

    mailbox->head = AppCtrlCommandMailbox_NextIndex(mailbox->head);
    mailbox->count--;

    return 1U;
}
