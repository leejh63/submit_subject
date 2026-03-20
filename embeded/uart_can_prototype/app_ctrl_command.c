#include "app_ctrl_command.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "can_types.h"

#define APP_CTRL_COMMAND_MAX_TOKENS  8U

static const AppCmdSpec g_appCmdSpecs[] =
{
    {"open",  APP_CTRL_COMMAND_OPEN,  APP_CMD_GRAMMAR_TARGET_ONLY,     1U, 1U},
    {"close", APP_CTRL_COMMAND_CLOSE, APP_CMD_GRAMMAR_TARGET_ONLY,     1U, 1U},
    {"off",   APP_CTRL_COMMAND_OFF,   APP_CMD_GRAMMAR_TARGET_ONLY,     1U, 1U},
    {"test",  APP_CTRL_COMMAND_TEST,  APP_CMD_GRAMMAR_TARGET_ONLY,     1U, 1U},
    {"text",  APP_CTRL_COMMAND_TEXT,  APP_CMD_GRAMMAR_TARGET_TEXT,     2U, APP_CMD_ARGS_UNBOUNDED},
    {"event", APP_CTRL_COMMAND_EVENT, APP_CMD_GRAMMAR_TARGET_U8_U8_U8, 4U, 4U}
};

static const char g_ctrlCmdHelpText[] =
    "open <id|all> close <id|all> off <id|all> test <id|all> "
    "text <id|all> <msg> event <id|all> <eventCode> <arg0> <arg1>";

static const AppCmdSpec *AppCtrlCommand_FindSpec(const char *name)
{
    size_t index;

    if (name == NULL || name[0] == '\0')
        return NULL;

    for (index = 0U; index < (sizeof(g_appCmdSpecs) / sizeof(g_appCmdSpecs[0])); index++)
    {
        if (strcmp(name, g_appCmdSpecs[index].name) == 0)
            return &g_appCmdSpecs[index];
    }

    return NULL;
}

static uint8_t AppCtrlCommand_IsSpaceChar(char ch)
{
    return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') ? 1U : 0U;
}

static uint8_t AppCtrlCommand_Tokenize(char *buffer,
                                       char *argv[],
                                       uint8_t argvCapacity,
                                       uint8_t *outArgc)
{
    uint8_t argc;
    char *cursor;

    if (buffer == NULL || argv == NULL || argvCapacity == 0U || outArgc == NULL)
        return 0U;

    argc = 0U;
    cursor = buffer;

    while (*cursor != '\0')
    {
        while (*cursor != '\0' && AppCtrlCommand_IsSpaceChar(*cursor) != 0U)
            cursor++;

        if (*cursor == '\0')
            break;

        if (argc >= argvCapacity)
            return 0U;

        argv[argc] = cursor;
        argc++;

        while (*cursor != '\0' && AppCtrlCommand_IsSpaceChar(*cursor) == 0U)
            cursor++;

        if (*cursor == '\0')
            break;

        *cursor = '\0';
        cursor++;
    }

    *outArgc = argc;
    return 1U;
}

static uint8_t AppCtrlCommand_ParseTarget(const char *text, AppCmdTarget *outTarget)
{
    long value;
    char *endPtr;

    if (text == NULL || outTarget == NULL)
        return 0U;

    if (text[0] == '\0')
        return 0U;

    if (strcmp(text, "all") == 0)
    {
        outTarget->hasValue = 1U;
        outTarget->nodeId = CAN_NODE_ID_BROADCAST;
        outTarget->isBroadcast = 1U;
        return 1U;
    }

    value = strtol(text, &endPtr, 10);
    if (*endPtr != '\0')
        return 0U;

    if (value < 1L || value > 255L)
        return 0U;

    outTarget->hasValue = 1U;
    outTarget->nodeId = (uint8_t)value;
    outTarget->isBroadcast = 0U;
    return 1U;
}

static uint8_t AppCtrlCommand_ParseU8(const char *text, uint8_t *outValue)
{
    long value;
    char *endPtr;

    if (text == NULL || outValue == NULL)
        return 0U;

    if (text[0] == '\0')
        return 0U;

    value = strtol(text, &endPtr, 10);
    if (*endPtr != '\0')
        return 0U;

    if (value < 0L || value > 255L)
        return 0U;

    *outValue = (uint8_t)value;
    return 1U;
}

static AppCtrlCommandParseResult AppCtrlCommand_ParseTargetOnly(const AppCmdSpec *spec,
                                                                uint8_t argc,
                                                                char *argv[],
                                                                AppCtrlCommand *outCmd)
{
    if (spec == NULL || argv == NULL || outCmd == NULL)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    if (argc != 2U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    outCmd->type = spec->type;
    if (AppCtrlCommand_ParseTarget(argv[1], &outCmd->target) == 0U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    return APP_CTRL_COMMAND_PARSE_OK;
}

static AppCtrlCommandParseResult AppCtrlCommand_ParseTargetText(const AppCmdSpec *spec,
                                                                uint8_t argc,
                                                                char *argv[],
                                                                AppCtrlCommand *outCmd)
{
    uint8_t tokenIndex;
    int written;
    int remaining;
    char *writePtr;

    if (spec == NULL || argv == NULL || outCmd == NULL)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    if (argc < 3U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    outCmd->type = spec->type;
    if (AppCtrlCommand_ParseTarget(argv[1], &outCmd->target) == 0U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    outCmd->payload.textArgs.text[0] = '\0';
    writePtr = outCmd->payload.textArgs.text;
    remaining = (int)sizeof(outCmd->payload.textArgs.text);

    for (tokenIndex = 2U; tokenIndex < argc; tokenIndex++)
    {
        if (tokenIndex > 2U)
        {
            if (remaining <= 1)
                return APP_CTRL_COMMAND_PARSE_INVALID;

            *writePtr = ' ';
            writePtr++;
            *writePtr = '\0';
            remaining--;
        }

        written = snprintf(writePtr, (size_t)remaining, "%s", argv[tokenIndex]);
        if (written <= 0 || written >= remaining)
            return APP_CTRL_COMMAND_PARSE_INVALID;

        writePtr += written;
        remaining -= written;
    }

    if (outCmd->payload.textArgs.text[0] == '\0')
        return APP_CTRL_COMMAND_PARSE_INVALID;

    return APP_CTRL_COMMAND_PARSE_OK;
}

static AppCtrlCommandParseResult AppCtrlCommand_ParseTargetU8U8U8(const AppCmdSpec *spec,
                                                                  uint8_t argc,
                                                                  char *argv[],
                                                                  AppCtrlCommand *outCmd)
{
    if (spec == NULL || argv == NULL || outCmd == NULL)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    if (argc != 5U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    outCmd->type = spec->type;

    if (AppCtrlCommand_ParseTarget(argv[1], &outCmd->target) == 0U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    if (AppCtrlCommand_ParseU8(argv[2], &outCmd->payload.eventArgs.eventCode) == 0U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    if (AppCtrlCommand_ParseU8(argv[3], &outCmd->payload.eventArgs.arg0) == 0U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    if (AppCtrlCommand_ParseU8(argv[4], &outCmd->payload.eventArgs.arg1) == 0U)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    return APP_CTRL_COMMAND_PARSE_OK;
}

static AppCtrlCommandParseResult AppCtrlCommand_ParseByGrammar(const AppCmdSpec *spec,
                                                               uint8_t argc,
                                                               char *argv[],
                                                               AppCtrlCommand *outCmd)
{
    if (spec == NULL || argv == NULL || outCmd == NULL)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    switch (spec->grammar)
    {
        case APP_CMD_GRAMMAR_TARGET_ONLY:
            return AppCtrlCommand_ParseTargetOnly(spec, argc, argv, outCmd);

        case APP_CMD_GRAMMAR_TARGET_TEXT:
            return AppCtrlCommand_ParseTargetText(spec, argc, argv, outCmd);

        case APP_CMD_GRAMMAR_TARGET_U8_U8_U8:
            return AppCtrlCommand_ParseTargetU8U8U8(spec, argc, argv, outCmd);

        case APP_CMD_GRAMMAR_LOCAL_ONLY:
        case APP_CMD_GRAMMAR_NONE:
        default:
            return APP_CTRL_COMMAND_PARSE_INVALID;
    }
}

static const char *AppCtrlCommand_TargetToString(const AppCmdTarget *target,
                                                 char               *buffer,
                                                 uint16_t            bufferSize)
{
    if (target == NULL)
        return "?";

    if (target->hasValue == 0U)
        return "default";

    if (buffer == NULL || bufferSize == 0U)
        return "?";

    if (target->isBroadcast != 0U || target->nodeId == CAN_NODE_ID_BROADCAST)
    {
        (void)snprintf(buffer, bufferSize, "all");
        return buffer;
    }

    (void)snprintf(buffer, bufferSize, "%u", (unsigned int)target->nodeId);
    return buffer;
}

void AppCtrlCommand_Clear(AppCtrlCommand *cmd)
{
    if (cmd == NULL)
        return;

    cmd->type = APP_CTRL_COMMAND_NONE;
    cmd->target.hasValue = 0U;
    cmd->target.nodeId = 0U;
    cmd->target.isBroadcast = 0U;
    cmd->payload.textArgs.text[0] = '\0';
}

AppCtrlCommandParseResult AppCtrlCommand_Parse(const char *input, AppCtrlCommand *outCmd)
{
    char parseBuffer[APP_CTRL_COMMAND_INPUT_SIZE];
    char *argv[APP_CTRL_COMMAND_MAX_TOKENS];
    uint8_t argc;
    uint8_t argCount;
    const AppCmdSpec *spec;

    if (input == NULL || outCmd == NULL)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    AppCtrlCommand_Clear(outCmd);

    (void)snprintf(parseBuffer, sizeof(parseBuffer), "%s", input);

    if (AppCtrlCommand_Tokenize(parseBuffer,
                                argv,
                                (uint8_t)(sizeof(argv) / sizeof(argv[0])),
                                &argc) == 0U)
    {
        return APP_CTRL_COMMAND_PARSE_INVALID;
    }

    if (argc == 0U)
        return APP_CTRL_COMMAND_PARSE_EMPTY;

    spec = AppCtrlCommand_FindSpec(argv[0]);
    if (spec == NULL)
        return APP_CTRL_COMMAND_PARSE_UNSUPPORTED;

    argCount = (uint8_t)(argc - 1U);
    if (argCount < spec->minArgs)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    if (spec->maxArgs != APP_CMD_ARGS_UNBOUNDED && argCount > spec->maxArgs)
        return APP_CTRL_COMMAND_PARSE_INVALID;

    return AppCtrlCommand_ParseByGrammar(spec, argc, argv, outCmd);
}

const char *AppCtrlCommand_TypeToString(AppCtrlCommandType type)
{
    switch (type)
    {
        case APP_CTRL_COMMAND_OPEN:
            return "open";
        case APP_CTRL_COMMAND_CLOSE:
            return "close";
        case APP_CTRL_COMMAND_OFF:
            return "off";
        case APP_CTRL_COMMAND_TEST:
            return "test";
        case APP_CTRL_COMMAND_TEXT:
            return "text";
        case APP_CTRL_COMMAND_EVENT:
            return "event";
        case APP_CTRL_COMMAND_NONE:
        default:
            return "none";
    }
}

const char *AppCtrlCommand_GetHelpText(void)
{
    return g_ctrlCmdHelpText;
}

uint8_t AppCtrlCommand_FormatSummary(const AppCtrlCommand *cmd,
                                     char                 *buffer,
                                     uint16_t              bufferSize,
                                     const char           *prefix)
{
    char targetText[8];
    const char *name;
    const char *prefixText;
    int written;

    if (cmd == NULL || buffer == NULL || bufferSize == 0U)
        return 0U;

    prefixText = (prefix != NULL) ? prefix : "";
    name = AppCtrlCommand_TypeToString(cmd->type);

    switch (cmd->type)
    {
        case APP_CTRL_COMMAND_TEXT:
            written = snprintf(buffer,
                               bufferSize,
                               "%s%s target=%s %s",
                               prefixText,
                               name,
                               AppCtrlCommand_TargetToString(&cmd->target,
                                                             targetText,
                                                             (uint16_t)sizeof(targetText)),
                               cmd->payload.textArgs.text);
            break;

        case APP_CTRL_COMMAND_EVENT:
            written = snprintf(buffer,
                               bufferSize,
                               "%s%s target=%s code=%u arg0=%u arg1=%u",
                               prefixText,
                               name,
                               AppCtrlCommand_TargetToString(&cmd->target,
                                                             targetText,
                                                             (uint16_t)sizeof(targetText)),
                               (unsigned int)cmd->payload.eventArgs.eventCode,
                               (unsigned int)cmd->payload.eventArgs.arg0,
                               (unsigned int)cmd->payload.eventArgs.arg1);
            break;

        default:
            written = snprintf(buffer,
                               bufferSize,
                               "%s%s target=%s",
                               prefixText,
                               name,
                               AppCtrlCommand_TargetToString(&cmd->target,
                                                             targetText,
                                                             (uint16_t)sizeof(targetText)));
            break;
    }

    if (written <= 0)
        return 0U;

    if ((uint16_t)written >= bufferSize)
        return 0U;

    return 1U;
}
