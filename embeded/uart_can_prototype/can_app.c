#include "can_app.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

static void CanApp_ClearCtrlResult(CtrlResult *result)
{
    CtrlResult_Clear(result);
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

static uint8_t CanApp_ResultQueuePush(CanApp *app, const CtrlResult *result)
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

static uint8_t CanApp_ResultQueuePop(CanApp *app, CtrlResult *outResult)
{
    if (app == NULL || outResult == NULL)
        return 0U;

    if (app->resultCount == 0U)
        return 0U;

    *outResult = app->resultQueue[app->resultHead];
    CanApp_ClearCtrlResult(&app->resultQueue[app->resultHead]);
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

static void CanApp_SetLocalTextResult(CanApp *app, CtrlResultType type, const char *text)
{
    CtrlResult result;

    if (app == NULL)
        return;

    CtrlResult_Clear(&result);
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
                                          const CtrlCmd *cmd,
                                          uint8_t *outTargetNodeId)
{
    if (app == NULL || cmd == NULL || outTargetNodeId == NULL)
        return 0U;

    if (cmd->hasTargetId != 0U)
    {
        if (CanApp_IsValidNodeId(cmd->targetId) == 0U)
            return 0U;

        *outTargetNodeId = cmd->targetId;
        return 1U;
    }

    if (CanApp_IsValidNodeId(app->defaultTargetNodeId) == 0U)
        return 0U;

    *outTargetNodeId = app->defaultTargetNodeId;
    return 1U;
}

static uint8_t CanApp_MapCtrlCmdToCanCommand(const CtrlCmd *cmd, uint8_t *outCommandCode)
{
    if (cmd == NULL || outCommandCode == NULL)
        return 0U;

    switch (cmd->type)
    {
        case CTRL_CMD_OPEN:
            *outCommandCode = CAN_CMD_OPEN;
            return 1U;
        case CTRL_CMD_CLOSE:
            *outCommandCode = CAN_CMD_CLOSE;
            return 1U;
        case CTRL_CMD_OFF:
            *outCommandCode = CAN_CMD_OFF;
            return 1U;
        case CTRL_CMD_TEST:
            *outCommandCode = CAN_CMD_TEST;
            return 1U;
        default:
            return 0U;
    }
}

static CtrlResultType CanApp_MapServiceResultType(uint8_t resultCode)
{
    switch (resultCode)
    {
        case CAN_RES_OK:
            return CTRL_RESULT_OK;
        case CAN_RES_TIMEOUT:
            return CTRL_RESULT_TIMEOUT;
        case CAN_RES_INVALID_TARGET:
            return CTRL_RESULT_INVALID_TARGET;
        default:
            return CTRL_RESULT_ERROR;
    }
}

static void CanApp_ConvertServiceResult(const CanServiceResult *serviceResult,
                                        CtrlResult *outResult)
{
    const char *commandName;

    if (serviceResult == NULL || outResult == NULL)
        return;

    CtrlResult_Clear(outResult);
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

static void CanApp_ConvertEvent(const CanMessage *message, CtrlResult *outResult)
{
    if (message == NULL || outResult == NULL)
        return;

    CtrlResult_Clear(outResult);
    outResult->type = CTRL_RESULT_OK;
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

static void CanApp_ConvertText(const CanMessage *message, CtrlResult *outResult)
{
    if (message == NULL || outResult == NULL)
        return;

    CtrlResult_Clear(outResult);
    outResult->type = CTRL_RESULT_OK;
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
    char text[CTRL_RESULT_TEXT_SIZE];

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

    CanApp_SetLocalTextResult(app, CTRL_RESULT_OK, text);
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
    CtrlResult ctrlResult;

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
    CtrlResult ctrlResult;

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
    CtrlResult ctrlResult;

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

uint8_t CanApp_Init(CanApp *app, const CanAppConfig *config)
{
    CanServiceConfig serviceConfig;

    if (app == NULL || config == NULL)
        return 0U;

    (void)memset(app, 0, sizeof(*app));
    app->localNodeId = config->localNodeId;
    app->role = config->role;
    app->defaultTargetNodeId = config->defaultTargetNodeId;

    serviceConfig.localNodeId = config->localNodeId;
    serviceConfig.instance = config->instance;
    serviceConfig.txMbIndex = config->txMbIndex;
    serviceConfig.rxMbIndex = config->rxMbIndex;
    serviceConfig.defaultTimeoutMs = config->defaultTimeoutMs;
    serviceConfig.driverState = config->driverState;
    serviceConfig.userConfig = config->userConfig;

    if (CanService_Init(&app->service, &serviceConfig) == 0U)
        return 0U;

    app->initialized = 1U;
    return 1U;
}

void CanApp_Task(CanApp *app, uint32_t nowMs)
{
    if (app == NULL || app->initialized == 0U)
        return;

    CanService_Task(&app->service, nowMs);
    CanApp_DrainRemoteCommands(app);
    CanApp_DrainServiceResults(app);
    CanApp_DrainEvents(app);
    CanApp_DrainTexts(app);
}

void CanApp_FlushTx(CanApp *app, uint32_t nowMs)
{
    if (app == NULL || app->initialized == 0U)
        return;

    CanService_FlushTx(&app->service, nowMs);
}

uint8_t CanApp_SubmitCtrlCmd(CanApp *app, const CtrlCmd *cmd)
{
    uint8_t targetNodeId;
    uint8_t commandCode;

    if (app == NULL || cmd == NULL || app->initialized == 0U)
        return 0U;

    if (CanApp_ResolveTargetNodeId(app, cmd, &targetNodeId) == 0U)
    {
        CanApp_SetLocalTextResult(app, CTRL_RESULT_INVALID_TARGET, "[error] invalid target");
        app->localSubmitFailCount++;
        return 0U;
    }

    if (cmd->type == CTRL_CMD_TEXT)
    {
        size_t textLen;

        if (cmd->hasText == 0U)
        {
            CanApp_SetLocalTextResult(app, CTRL_RESULT_ERROR, "[error] empty text");
            app->localSubmitFailCount++;
            return 0U;
        }

        textLen = strlen(cmd->text);
        if (textLen > CAN_TEXT_MAX_LEN)
        {
            char text[CTRL_RESULT_TEXT_SIZE];

            (void)snprintf(text,
                           sizeof(text),
                           "[error] text too long max=%u",
                           (unsigned int)CAN_TEXT_MAX_LEN);
            CanApp_SetLocalTextResult(app, CTRL_RESULT_ERROR, text);
            app->localSubmitFailCount++;
            return 0U;
        }

        if (CanService_SendText(&app->service, targetNodeId, CAN_TEXT_USER, cmd->text) == 0U)
        {
            CanApp_SetLocalTextResult(app, CTRL_RESULT_ERROR, "[error] can text send failed");
            app->localSubmitFailCount++;
            return 0U;
        }

        app->localSubmitOkCount++;
        return 1U;
    }

    if (CanApp_MapCtrlCmdToCanCommand(cmd, &commandCode) == 0U)
    {
        CanApp_SetLocalTextResult(app, CTRL_RESULT_ERROR, "[error] unsupported ctrl cmd");
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
        CanApp_SetLocalTextResult(app, CTRL_RESULT_ERROR, "[error] can send failed");
        app->localSubmitFailCount++;
        return 0U;
    }

    app->localSubmitOkCount++;
    return 1U;
}

uint8_t CanApp_PopCtrlResult(CanApp *app, CtrlResult *outResult)
{
    return CanApp_ResultQueuePop(app, outResult);
}
