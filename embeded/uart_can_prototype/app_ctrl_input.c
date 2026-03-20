#include "app_ctrl_input.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "app_ctrl_command.h"

typedef void (*AppCtrlInputLocalHandler)(const AppCtrlInputSnapshot *snapshot,
                                      AppCtrlInputResult         *outResult);

typedef struct
{
    const char            *name;
    AppCtrlInputLocalHandler  handler;
} AppCtrlInputLocalCommand;

static void AppCtrlInput_SetText(AppCtrlInputResult *result, const char *text)
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

static void AppCtrlInput_LocalHelp(const AppCtrlInputSnapshot *snapshot,
                                AppCtrlInputResult         *outResult)
{
    if (outResult == NULL)
        return;

    (void)snapshot;

    (void)snprintf(outResult->text,
                   sizeof(outResult->text),
                   "cmd: help hello ping status %s",
                   AppCtrlCommand_GetHelpText());
}

static void AppCtrlInput_LocalHello(const AppCtrlInputSnapshot *snapshot,
                                 AppCtrlInputResult         *outResult)
{
    (void)snapshot;
    AppCtrlInput_SetText(outResult, "hello");
}

static void AppCtrlInput_LocalPing(const AppCtrlInputSnapshot *snapshot,
                                AppCtrlInputResult         *outResult)
{
    (void)snapshot;
    AppCtrlInput_SetText(outResult, "pong");
}

static void AppCtrlInput_LocalStatus(const AppCtrlInputSnapshot *snapshot,
                                  AppCtrlInputResult         *outResult)
{
    if (snapshot == NULL || outResult == NULL)
        return;

    (void)snprintf(outResult->text,
                   sizeof(outResult->text),
                   "state=%u uart_err=%u rx_len=%u tx_busy=%u cmd_q=%u/%u result_q=%u/%u",
                   (unsigned int)snapshot->consoleState,
                   (unsigned int)snapshot->uartErrorFlag,
                   (unsigned int)snapshot->rxLineLength,
                   (unsigned int)snapshot->txBusy,
                   (unsigned int)snapshot->commandQueueCount,
                   (unsigned int)snapshot->commandQueueCapacity,
                   (unsigned int)snapshot->resultQueueCount,
                   (unsigned int)snapshot->resultQueueCapacity);
}

static const AppCtrlInputLocalCommand g_ctrlInputLocalCommands[] =
{
    {"help",   AppCtrlInput_LocalHelp},
    {"hello",  AppCtrlInput_LocalHello},
    {"ping",   AppCtrlInput_LocalPing},
    {"status", AppCtrlInput_LocalStatus}
};

static uint8_t AppCtrlInput_TryHandleLocalCommand(const char              *line,
                                               const AppCtrlInputSnapshot *snapshot,
                                               AppCtrlInputResult         *outResult)
{
    size_t index;

    if (line == NULL || snapshot == NULL || outResult == NULL)
        return 0U;

    for (index = 0U; index < (sizeof(g_ctrlInputLocalCommands) / sizeof(g_ctrlInputLocalCommands[0])); index++)
    {
        if (strcmp(line, g_ctrlInputLocalCommands[index].name) == 0)
        {
            g_ctrlInputLocalCommands[index].handler(snapshot, outResult);
            return 1U;
        }
    }

    return 0U;
}

static status_t AppCtrlInput_HandleRemoteCommand(const char      *line,
                                              AppCtrlCommandMailbox     *mailbox,
                                              AppCtrlInputResult *outResult)
{
    AppCtrlCommand       cmd;
    AppCtrlCommandParseResult parseResult;
    uint8_t       pushOk;

    if (line == NULL || mailbox == NULL || outResult == NULL)
        return STATUS_ERROR;

    parseResult = AppCtrlCommand_Parse(line, &cmd);
    if (parseResult == APP_CTRL_COMMAND_PARSE_EMPTY)
        return STATUS_SUCCESS;

    if (parseResult == APP_CTRL_COMMAND_PARSE_INVALID)
    {
        AppCtrlInput_SetText(outResult, "[error] invalid command");
        return STATUS_ERROR;
    }

    if (parseResult == APP_CTRL_COMMAND_PARSE_UNSUPPORTED)
    {
        AppCtrlInput_SetText(outResult, "[error] unsupported command");
        return STATUS_ERROR;
    }

    pushOk = AppCtrlCommandMailbox_Push(mailbox, &cmd);
    if (pushOk == 0U)
    {
        AppCtrlInput_SetText(outResult, "[busy] can cmd queue full");
        return STATUS_BUSY;
    }

    if (AppCtrlCommand_FormatSummary(&cmd,
                              outResult->text,
                              (uint16_t)sizeof(outResult->text),
                              "[queued] ") == 0U)
    {
        AppCtrlInput_SetText(outResult, "[queued] command");
    }

    return STATUS_SUCCESS;
}

void AppCtrlInputResult_Clear(AppCtrlInputResult *result)
{
    if (result == NULL)
        return;

    result->text[0] = '\0';
}

status_t AppCtrlInput_HandleLine(const char              *line,
                              const AppCtrlInputSnapshot *snapshot,
                              AppCtrlCommandMailbox             *mailbox,
                              AppCtrlInputResult         *outResult)
{
    if (line == NULL || snapshot == NULL || mailbox == NULL || outResult == NULL)
        return STATUS_ERROR;

    AppCtrlInputResult_Clear(outResult);

    if (AppCtrlInput_TryHandleLocalCommand(line, snapshot, outResult) != 0U)
        return STATUS_SUCCESS;

    return AppCtrlInput_HandleRemoteCommand(line, mailbox, outResult);
}
