#include "can_app.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>



typedef struct
{
    uint8_t                     instance;
    uint8_t                     txMbIndex;
    uint8_t                     rxMbIndex;
    uint32_t                    defaultTimeoutMs;
    flexcan_state_t            *driverState;
    const flexcan_user_config_t *userConfig;
} CanAppHwConfig;

static const CanAppHwConfig g_canAppHwConfig =
{
    INST_FLEXCAN_CONFIG_1,
    CAN_HW_TX_MB_INDEX,
    CAN_HW_RX_MB_INDEX,
    300U,
    &flexcanState0,
    &flexcanInitConfig0
};

static void CanApp_ClearAppCtrlResult(AppCtrlResult *result)
{
    AppCtrlResult_Clear(result);
}

static uint8_t CanApp_ResultNext(uint8_t index)
{
    index++;
    if (index >= CAN_APP_RESULT_QUEUE_SIZE)
        index = 0U;
    return index;
}

static uint8_t CanApp_ResultQueueIsFull(const CanApp *app)
{
    if (app == NULL)
        return 1U;

    return (app->resultCount >= CAN_APP_RESULT_QUEUE_SIZE) ? 1U : 0U;
}

static uint8_t CanApp_ResultQueuePush(CanApp *app, const AppCtrlResult *result)
{
    if (app == NULL || result == NULL)
        return 0U;

    if (CanApp_ResultQueueIsFull(app) != 0U)
    {
        app->resultDropCount++;
        return 0U;
    }

    app->resultQueue[app->resultTail] = *result;
    app->resultTail = CanApp_ResultNext(app->resultTail);
    app->resultCount++;
    return 1U;
}

static uint8_t CanApp_ResultQueuePop(CanApp *app, AppCtrlResult *outResult)
{
    if (app == NULL || outResult == NULL)
        return 0U;

    if (app->resultCount == 0U)
        return 0U;

    *outResult = app->resultQueue[app->resultHead];
    CanApp_ClearAppCtrlResult(&app->resultQueue[app->resultHead]);
    app->resultHead = CanApp_ResultNext(app->resultHead);
    app->resultCount--;
    return 1U;
}

static const char *CanApp_CommandName(uint8_t commandCode)
{
    switch (commandCode)
    {
        case CAN_CMD_OPEN:
            return "open";
        case CAN_CMD_CLOSE:
            return "close";
        case CAN_CMD_OFF:
            return "off";
        case CAN_CMD_TEST:
            return "test";
        default:
            return "unknown";
    }
}

static void CanApp_SetLocalTextResult(CanApp *app, AppCtrlResultType type, const char *text)
{
    AppCtrlResult result;

    if (app == NULL)
        return;

    AppCtrlResult_Clear(&result);
    result.type = type;
    (void)snprintf(result.text, sizeof(result.text), "%s", (text != NULL) ? text : "");
    (void)CanApp_ResultQueuePush(app, &result);
}

static uint8_t CanApp_IsValidNodeId(uint8_t nodeId)
{
    if (nodeId == CAN_NODE_ID_BROADCAST)
        return 1U;

    if (nodeId < CAN_NODE_ID_MIN || nodeId > CAN_NODE_ID_MAX)
        return 0U;

    return 1U;
}

static uint8_t CanApp_ResolveTargetNodeId(const CanApp *app,
                                          const AppCtrlCommand *cmd,
                                          uint8_t *outTargetNodeId)
{
    if (app == NULL || cmd == NULL || outTargetNodeId == NULL)
        return 0U;

    if (cmd->target.hasValue != 0U)
    {
        if (CanApp_IsValidNodeId(cmd->target.nodeId) == 0U)
            return 0U;

        *outTargetNodeId = cmd->target.nodeId;
        return 1U;
    }

    if (CanApp_IsValidNodeId(app->defaultTargetNodeId) == 0U)
        return 0U;

    *outTargetNodeId = app->defaultTargetNodeId;
    return 1U;
}

static uint8_t CanApp_MapAppCtrlCommandToCanCommand(const AppCtrlCommand *cmd, uint8_t *outCommandCode)
{
    if (cmd == NULL || outCommandCode == NULL)
        return 0U;

    switch (cmd->type)
    {
        case APP_CTRL_COMMAND_OPEN:
            *outCommandCode = CAN_CMD_OPEN;
            return 1U;
        case APP_CTRL_COMMAND_CLOSE:
            *outCommandCode = CAN_CMD_CLOSE;
            return 1U;
        case APP_CTRL_COMMAND_OFF:
            *outCommandCode = CAN_CMD_OFF;
            return 1U;
        case APP_CTRL_COMMAND_TEST:
            *outCommandCode = CAN_CMD_TEST;
            return 1U;
        default:
            return 0U;
    }
}

static AppCtrlResultType CanApp_MapServiceResultType(uint8_t resultCode)
{
    switch (resultCode)
    {
        case CAN_RES_OK:
            return APP_CTRL_RESULT_OK;
        case CAN_RES_TIMEOUT:
            return APP_CTRL_RESULT_TIMEOUT;
        case CAN_RES_INVALID_TARGET:
            return APP_CTRL_RESULT_INVALID_TARGET;
        default:
            return APP_CTRL_RESULT_ERROR;
    }
}

static void CanApp_ConvertServiceResult(const CanServiceResult *serviceResult,
                                        AppCtrlResult *outResult)
{
    const char *commandName;

    if (serviceResult == NULL || outResult == NULL)
        return;

    AppCtrlResult_Clear(outResult);
    outResult->type = CanApp_MapServiceResultType(serviceResult->resultCode);
    outResult->hasTargetId = 1U;
    outResult->targetId = serviceResult->sourceNodeId;

    commandName = CanApp_CommandName(serviceResult->commandCode);

    if (serviceResult->kind == CAN_SERVICE_RESULT_TIMEOUT)
    {
        (void)snprintf(outResult->text,
                       sizeof(outResult->text),
                       "[timeout] %s target=%u",
                       commandName,
                       (unsigned int)serviceResult->sourceNodeId);
        return;
    }

    switch (serviceResult->resultCode)
    {
        case CAN_RES_OK:
            (void)snprintf(outResult->text,
                           sizeof(outResult->text),
                           "[ok] %s target=%u",
                           commandName,
                           (unsigned int)serviceResult->sourceNodeId);
            break;
        case CAN_RES_INVALID_TARGET:
            (void)snprintf(outResult->text,
                           sizeof(outResult->text),
                           "[error] invalid target=%u",
                           (unsigned int)serviceResult->sourceNodeId);
            break;
        case CAN_RES_NOT_SUPPORTED:
            (void)snprintf(outResult->text,
                           sizeof(outResult->text),
                           "[error] %s not supported target=%u",
                           commandName,
                           (unsigned int)serviceResult->sourceNodeId);
            break;
        default:
            (void)snprintf(outResult->text,
                           sizeof(outResult->text),
                           "[error] %s target=%u code=%u",
                           commandName,
                           (unsigned int)serviceResult->sourceNodeId,
                           (unsigned int)serviceResult->resultCode);
            break;
    }
}

static void CanApp_ConvertEvent(const CanMessage *message, AppCtrlResult *outResult)
{
    if (message == NULL || outResult == NULL)
        return;

    AppCtrlResult_Clear(outResult);
    outResult->type = APP_CTRL_RESULT_OK;
    outResult->hasTargetId = 1U;
    outResult->targetId = message->sourceNodeId;

    (void)snprintf(outResult->text,
                   sizeof(outResult->text),
                   "[event] code=%u from=%u arg0=%u arg1=%u",
                   (unsigned int)message->payload[0],
                   (unsigned int)message->sourceNodeId,
                   (unsigned int)message->payload[1],
                   (unsigned int)message->payload[2]);
}

static void CanApp_ConvertText(const CanMessage *message, AppCtrlResult *outResult)
{
    if (message == NULL || outResult == NULL)
        return;

    AppCtrlResult_Clear(outResult);
    outResult->type = APP_CTRL_RESULT_OK;
    outResult->hasTargetId = 1U;
    outResult->targetId = message->sourceNodeId;

    (void)snprintf(outResult->text,
                   sizeof(outResult->text),
                   "[text] from=%u %s",
                   (unsigned int)message->sourceNodeId,
                   message->text);
}

static void CanApp_HandleRemoteCommand(CanApp *app, const CanMessage *message)
{
    uint8_t commandCode;
    uint8_t resultCode;
    char text[APP_CTRL_RESULT_TEXT_SIZE];

    if (app == NULL || message == NULL)
        return;

    commandCode = message->payload[0];
    resultCode = CAN_RES_OK;

    switch (commandCode)
    {
        case CAN_CMD_OPEN:
        case CAN_CMD_CLOSE:
        case CAN_CMD_OFF:
        case CAN_CMD_TEST:
            (void)snprintf(text,
                           sizeof(text),
                           "[remote] %s from=%u",
                           CanApp_CommandName(commandCode),
                           (unsigned int)message->sourceNodeId);
            break;
        default:
            resultCode = CAN_RES_NOT_SUPPORTED;
            (void)snprintf(text,
                           sizeof(text),
                           "[remote] unsupported cmd=%u from=%u",
                           (unsigned int)commandCode,
                           (unsigned int)message->sourceNodeId);
            break;
    }

    CanApp_SetLocalTextResult(app, APP_CTRL_RESULT_OK, text);
    app->remoteCommandCount++;

    if ((message->flags & CAN_MSG_FLAG_NEED_RESPONSE) != 0U && message->requestId != 0U)
    {
        (void)CanService_SendResponse(&app->service,
                                      message->sourceNodeId,
                                      message->requestId,
                                      resultCode,
                                      0U);
    }
}

static void CanApp_DrainServiceResults(CanApp *app)
{
    CanServiceResult serviceResult;
    AppCtrlResult ctrlResult;

    if (app == NULL)
        return;

    while (CanService_PopResult(&app->service, &serviceResult) != 0U)
    {
        CanApp_ConvertServiceResult(&serviceResult, &ctrlResult);
        (void)CanApp_ResultQueuePush(app, &ctrlResult);
        app->resultConvertCount++;
    }
}

static void CanApp_DrainEvents(CanApp *app)
{
    CanMessage eventMessage;
    AppCtrlResult ctrlResult;

    if (app == NULL)
        return;

    while (CanService_PopEvent(&app->service, &eventMessage) != 0U)
    {
        CanApp_ConvertEvent(&eventMessage, &ctrlResult);
        (void)CanApp_ResultQueuePush(app, &ctrlResult);
        app->eventConvertCount++;
    }
}

static void CanApp_DrainTexts(CanApp *app)
{
    CanMessage textMessage;
    AppCtrlResult ctrlResult;

    if (app == NULL)
        return;

    while (CanService_PopText(&app->service, &textMessage) != 0U)
    {
        CanApp_ConvertText(&textMessage, &ctrlResult);
        (void)CanApp_ResultQueuePush(app, &ctrlResult);
        app->textConvertCount++;
    }
}

static void CanApp_DrainRemoteCommands(CanApp *app)
{
    CanMessage commandMessage;

    if (app == NULL)
        return;

    while (CanService_PopReceivedCommand(&app->service, &commandMessage) != 0U)
        CanApp_HandleRemoteCommand(app, &commandMessage);
}

uint8_t CanApp_Init(CanApp *app,
                    uint8_t localNodeId,
                    uint8_t role,
                    uint8_t defaultTargetNodeId)
{
    CanServiceConfig serviceConfig;

    if (app == NULL)
        return 0U;

    if ((g_canAppHwConfig.driverState == NULL) ||
        (g_canAppHwConfig.userConfig == NULL))
        return 0U;

    (void)memset(app, 0, sizeof(*app));
    app->localNodeId = localNodeId;
    app->role = role;
    app->defaultTargetNodeId = defaultTargetNodeId;

    serviceConfig.localNodeId = localNodeId;
    serviceConfig.instance = g_canAppHwConfig.instance;
    serviceConfig.txMbIndex = g_canAppHwConfig.txMbIndex;
    serviceConfig.rxMbIndex = g_canAppHwConfig.rxMbIndex;
    serviceConfig.defaultTimeoutMs = g_canAppHwConfig.defaultTimeoutMs;
    serviceConfig.driverState = g_canAppHwConfig.driverState;
    serviceConfig.userConfig = g_canAppHwConfig.userConfig;

    if (CanService_Init(&app->service, &serviceConfig) == 0U)
        return 0U;

    app->initialized = 1U;
    return 1U;
}

void CanApp_RunService(CanApp *app, uint32_t nowMs)
{
    if (app == NULL || app->initialized == 0U)
        return;

    CanService_Task(&app->service, nowMs);
}

void CanApp_RunRemoteCommands(CanApp *app)
{
    if (app == NULL || app->initialized == 0U)
        return;

    CanApp_DrainRemoteCommands(app);
}

void CanApp_RunServiceResults(CanApp *app)
{
    if (app == NULL || app->initialized == 0U)
        return;

    CanApp_DrainServiceResults(app);
}

void CanApp_RunEvents(CanApp *app)
{
    if (app == NULL || app->initialized == 0U)
        return;

    CanApp_DrainEvents(app);
}

void CanApp_RunTexts(CanApp *app)
{
    if (app == NULL || app->initialized == 0U)
        return;

    CanApp_DrainTexts(app);
}

void CanApp_FlushTx(CanApp *app, uint32_t nowMs)
{
    if (app == NULL || app->initialized == 0U)
        return;

    CanService_FlushTx(&app->service, nowMs);
}

uint8_t CanApp_SubmitAppCtrlCommand(CanApp *app, const AppCtrlCommand *cmd)
{
    uint8_t targetNodeId;
    uint8_t commandCode;

    if (app == NULL || cmd == NULL || app->initialized == 0U)
        return 0U;

    if (CanApp_ResolveTargetNodeId(app, cmd, &targetNodeId) == 0U)
    {
        CanApp_SetLocalTextResult(app, APP_CTRL_RESULT_INVALID_TARGET, "[error] invalid target");
        app->localSubmitFailCount++;
        return 0U;
    }
// 이것도 중간에 스위치문으로 헬퍼함수로 따로 빼도 문제 없을듯?
    if (cmd->type == APP_CTRL_COMMAND_TEXT)
    {
        size_t textLen;
        const char *textValue;

        textValue = cmd->payload.textArgs.text;
        if (textValue[0] == '\0')
        {
            CanApp_SetLocalTextResult(app, APP_CTRL_RESULT_ERROR, "[error] empty text");
            app->localSubmitFailCount++;
            return 0U;
        }

        textLen = strlen(textValue);
        if (textLen > CAN_TEXT_MAX_LEN)
        {
            char text[APP_CTRL_RESULT_TEXT_SIZE];

            (void)snprintf(text,
                           sizeof(text),
                           "[error] text too long max=%u",
                           (unsigned int)CAN_TEXT_MAX_LEN);
            CanApp_SetLocalTextResult(app, APP_CTRL_RESULT_ERROR, text);
            app->localSubmitFailCount++;
            return 0U;
        }

        if (CanService_SendText(&app->service, targetNodeId, CAN_TEXT_USER, textValue) == 0U)
        {
            CanApp_SetLocalTextResult(app, APP_CTRL_RESULT_ERROR, "[error] can text send failed");
            app->localSubmitFailCount++;
            return 0U;
        }

        app->localSubmitOkCount++;
        return 1U;
    }

    if (cmd->type == APP_CTRL_COMMAND_EVENT)
    {
        if (CanService_SendEvent(&app->service,
                                 targetNodeId,
                                 cmd->payload.eventArgs.eventCode,
                                 cmd->payload.eventArgs.arg0,
                                 cmd->payload.eventArgs.arg1) == 0U)
        {
            CanApp_SetLocalTextResult(app, APP_CTRL_RESULT_ERROR, "[error] can event send failed");
            app->localSubmitFailCount++;
            return 0U;
        }

        app->localSubmitOkCount++;
        return 1U;
    }

    if (CanApp_MapAppCtrlCommandToCanCommand(cmd, &commandCode) == 0U)
    {
        CanApp_SetLocalTextResult(app, APP_CTRL_RESULT_ERROR, "[error] unsupported ctrl cmd");
        app->localSubmitFailCount++;
        return 0U;
    }

    if (CanService_SendCommand(&app->service,
                               targetNodeId,
                               commandCode,
                               0U,
                               0U,
                               1U) == 0U)
    {
        CanApp_SetLocalTextResult(app, APP_CTRL_RESULT_ERROR, "[error] can send failed");
        app->localSubmitFailCount++;
        return 0U;
    }

    app->localSubmitOkCount++;
    return 1U;
}

uint8_t CanApp_PopAppCtrlResult(CanApp *app, AppCtrlResult *outResult)
{
    return CanApp_ResultQueuePop(app, outResult);
}
