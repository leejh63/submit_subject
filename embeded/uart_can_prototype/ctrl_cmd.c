#include "ctrl_cmd.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "can_types.h"

static uint8_t CtrlCmd_ParseTargetId(const char *text, uint8_t *outTargetId)
{
    long value;
    char *endPtr;

    if (text == NULL || outTargetId == NULL)
        return 0U;

    if (text[0] == '\0')
        return 0U;

    if (strcmp(text, "all") == 0)
    {
        *outTargetId = CAN_NODE_ID_BROADCAST;
        return 1U;
    }

    value = strtol(text, &endPtr, 10);
    if (*endPtr != '\0')
        return 0U;

    if (value < 1L || value > 255L)
        return 0U;

    *outTargetId = (uint8_t)value;
    return 1U;
}

void CtrlCmd_Clear(CtrlCmd *cmd)
{
    if (cmd == NULL)
        return;

    cmd->type = CTRL_CMD_NONE;
    cmd->hasTargetId = 0U;
    cmd->targetId = 0U;
    cmd->hasText = 0U;
    cmd->text[0] = '\0';
    cmd->hasAuthText = 0U;
    cmd->authText[0] = '\0';
    cmd->hasRequestId = 0U;
    cmd->requestId = 0U;
}

CtrlCmdResult CtrlCmd_Parse(const char *input, CtrlCmd *outCmd)
{
    char cmd[16];
    char arg1[16];
    char arg2[CTRL_CMD_TEXT_SIZE];
    int  count;

    if (input == NULL || outCmd == NULL)
        return CTRL_CMD_RESULT_INVALID;

    CtrlCmd_Clear(outCmd);

    cmd[0] = '\0';
    arg1[0] = '\0';
    arg2[0] = '\0';

    count = sscanf(input, "%15s %15s %31[^\n]", cmd, arg1, arg2);
    if (count <= 0)
        return CTRL_CMD_RESULT_EMPTY;

    if (strcmp(cmd, "open") == 0)
    {
        outCmd->type = CTRL_CMD_OPEN;
        if (count < 2)
            return CTRL_CMD_RESULT_INVALID;

        if (CtrlCmd_ParseTargetId(arg1, &outCmd->targetId) == 0U)
            return CTRL_CMD_RESULT_INVALID;

        outCmd->hasTargetId = 1U;
        return CTRL_CMD_RESULT_OK;
    }

    if (strcmp(cmd, "close") == 0)
    {
        outCmd->type = CTRL_CMD_CLOSE;
        if (count < 2)
            return CTRL_CMD_RESULT_INVALID;

        if (CtrlCmd_ParseTargetId(arg1, &outCmd->targetId) == 0U)
            return CTRL_CMD_RESULT_INVALID;

        outCmd->hasTargetId = 1U;
        return CTRL_CMD_RESULT_OK;
    }

    if (strcmp(cmd, "off") == 0)
    {
        outCmd->type = CTRL_CMD_OFF;
        if (count < 2)
            return CTRL_CMD_RESULT_INVALID;

        if (CtrlCmd_ParseTargetId(arg1, &outCmd->targetId) == 0U)
            return CTRL_CMD_RESULT_INVALID;

        outCmd->hasTargetId = 1U;
        return CTRL_CMD_RESULT_OK;
    }

    if (strcmp(cmd, "test") == 0)
    {
        outCmd->type = CTRL_CMD_TEST;
        if (count < 2)
            return CTRL_CMD_RESULT_INVALID;

        if (CtrlCmd_ParseTargetId(arg1, &outCmd->targetId) == 0U)
            return CTRL_CMD_RESULT_INVALID;

        outCmd->hasTargetId = 1U;
        return CTRL_CMD_RESULT_OK;
    }

    if (strcmp(cmd, "text") == 0)
    {
        outCmd->type = CTRL_CMD_TEXT;

        if (count < 3)
            return CTRL_CMD_RESULT_INVALID;

        if (CtrlCmd_ParseTargetId(arg1, &outCmd->targetId) == 0U)
            return CTRL_CMD_RESULT_INVALID;

        outCmd->hasTargetId = 1U;

        if (arg2[0] == '\0')
            return CTRL_CMD_RESULT_INVALID;

        outCmd->hasText = 1U;
        (void)snprintf(outCmd->text, sizeof(outCmd->text), "%s", arg2);
        return CTRL_CMD_RESULT_OK;
    }

    return CTRL_CMD_RESULT_UNSUPPORTED;
}
