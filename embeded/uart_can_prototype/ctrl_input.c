#include "ctrl_input.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "ctrl_cmd.h"
#include "can_types.h"

static void CtrlInput_SetText(CtrlInputResult *result, const char *text)
{
    if (result == NULL)
        return;

    if (text == NULL)
    {
        result->text[0] = '\0';
        return;
    }

    (void)snprintf(result->text, sizeof(result->text), "%s", text);
}

static const char *CtrlInput_TargetText(uint8_t targetId)
{
    if (targetId == CAN_NODE_ID_BROADCAST)
        return "all";

    return NULL;
}

static uint8_t CtrlInput_HandleLocalCommand(const char              *line,
                                            const CtrlInputSnapshot *snapshot,
                                            CtrlInputResult         *outResult)
{
    if (line == NULL || snapshot == NULL || outResult == NULL)
        return 0U;

    if (strcmp(line, "help") == 0)
    {
        CtrlInput_SetText(outResult,
                          "cmd: help hello ping status open <id|all> close <id|all> off <id|all> test <id|all> text <id|all> <msg>");
        return 1U;
    }

    if (strcmp(line, "hello") == 0)
    {
        CtrlInput_SetText(outResult, "hello");
        return 1U;
    }

    if (strcmp(line, "ping") == 0)
    {
        CtrlInput_SetText(outResult, "pong");
        return 1U;
    }

    if (strcmp(line, "status") == 0)
    {
        (void)snprintf(outResult->text,
                       sizeof(outResult->text),
                       "node=%u uart_err=%u rx_len=%u tx_busy=%u cmd_q=%u/%u result_q=%u/%u",
                       (unsigned int)snapshot->nodeState,
                       (unsigned int)snapshot->uartErrorFlag,
                       (unsigned int)snapshot->rxLineLength,
                       (unsigned int)snapshot->txBusy,
                       (unsigned int)snapshot->cmdQueueCount,
                       (unsigned int)snapshot->cmdQueueCapacity,
                       (unsigned int)snapshot->resultQueueCount,
                       (unsigned int)snapshot->resultQueueCapacity);
        return 1U;
    }

    return 0U;
}

void CtrlInputResult_Clear(CtrlInputResult *result)
{
    if (result == NULL)
        return;

    result->text[0] = '\0';
}

status_t CtrlInput_HandleLine(const char              *line,
                              const CtrlInputSnapshot *snapshot,
                              CtrlMailbox             *mailbox,
                              CtrlInputResult         *outResult)
{
    CtrlCmd       cmd;
    CtrlCmdResult parseResult;
    uint8_t       pushOk;

    if (line == NULL || snapshot == NULL || mailbox == NULL || outResult == NULL)
        return STATUS_ERROR;

    CtrlInputResult_Clear(outResult);

    if (CtrlInput_HandleLocalCommand(line, snapshot, outResult) != 0U)
        return STATUS_SUCCESS;

    parseResult = CtrlCmd_Parse(line, &cmd);
    if (parseResult == CTRL_CMD_RESULT_EMPTY)
        return STATUS_SUCCESS;

    if (parseResult == CTRL_CMD_RESULT_INVALID)
    {
        CtrlInput_SetText(outResult, "[error] invalid command");
        return STATUS_ERROR;
    }

    if (parseResult == CTRL_CMD_RESULT_UNSUPPORTED)
    {
        CtrlInput_SetText(outResult, "[error] unsupported command");
        return STATUS_ERROR;
    }

    pushOk = CtrlMailbox_Push(mailbox, &cmd);
    if (pushOk == 0U)
    {
        CtrlInput_SetText(outResult, "[busy] can cmd queue full");
        return STATUS_BUSY;
    }

    switch (cmd.type)
    {
        case CTRL_CMD_OPEN:
            if (cmd.hasTargetId != 0U)
            {
                if (CtrlInput_TargetText(cmd.targetId) != NULL)
                    (void)snprintf(outResult->text, sizeof(outResult->text), "[queued] open target=%s",
                                   CtrlInput_TargetText(cmd.targetId));
                else
                    (void)snprintf(outResult->text, sizeof(outResult->text), "[queued] open target=%u",
                                   (unsigned int)cmd.targetId);
            }
            else
                CtrlInput_SetText(outResult, "[queued] open");
            break;

        case CTRL_CMD_CLOSE:
            if (cmd.hasTargetId != 0U)
            {
                if (CtrlInput_TargetText(cmd.targetId) != NULL)
                    (void)snprintf(outResult->text, sizeof(outResult->text), "[queued] close target=%s",
                                   CtrlInput_TargetText(cmd.targetId));
                else
                    (void)snprintf(outResult->text, sizeof(outResult->text), "[queued] close target=%u",
                                   (unsigned int)cmd.targetId);
            }
            else
                CtrlInput_SetText(outResult, "[queued] close");
            break;

        case CTRL_CMD_OFF:
            if (cmd.hasTargetId != 0U)
            {
                if (CtrlInput_TargetText(cmd.targetId) != NULL)
                    (void)snprintf(outResult->text, sizeof(outResult->text), "[queued] off target=%s",
                                   CtrlInput_TargetText(cmd.targetId));
                else
                    (void)snprintf(outResult->text, sizeof(outResult->text), "[queued] off target=%u",
                                   (unsigned int)cmd.targetId);
            }
            else
                CtrlInput_SetText(outResult, "[queued] off");
            break;

        case CTRL_CMD_TEST:
            if (cmd.hasTargetId != 0U)
            {
                if (CtrlInput_TargetText(cmd.targetId) != NULL)
                    (void)snprintf(outResult->text, sizeof(outResult->text), "[queued] test target=%s",
                                   CtrlInput_TargetText(cmd.targetId));
                else
                    (void)snprintf(outResult->text, sizeof(outResult->text), "[queued] test target=%u",
                                   (unsigned int)cmd.targetId);
            }
            else
                CtrlInput_SetText(outResult, "[queued] test");
            break;

        case CTRL_CMD_TEXT:
            if (CtrlInput_TargetText(cmd.targetId) != NULL)
                (void)snprintf(outResult->text,
                               sizeof(outResult->text),
                               "[queued] text target=%s %s",
                               CtrlInput_TargetText(cmd.targetId),
                               cmd.text);
            else
                (void)snprintf(outResult->text,
                               sizeof(outResult->text),
                               "[queued] text target=%u %s",
                               (unsigned int)cmd.targetId,
                               cmd.text);
            break;

        default:
            CtrlInput_SetText(outResult, "[error] command state");
            return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}
