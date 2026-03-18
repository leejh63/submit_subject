#include "can_service.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

static void CanService_ClearMessage(CanMessage *message)
{
    if (message == NULL)
        return;

    (void)memset(message, 0, sizeof(*message));
}

static void CanService_ClearResult(CanServiceResult *result)
{
    if (result == NULL)
        return;

    (void)memset(result, 0, sizeof(*result));
}

static void CanService_ClearPending(CanPendingRequest *pending)
{
    if (pending == NULL)
        return;

    (void)memset(pending, 0, sizeof(*pending));
}

static uint8_t CanService_IsValidTarget(uint8_t nodeId)
{
    if (nodeId == CAN_NODE_ID_BROADCAST)
        return 1U;

    if (nodeId < CAN_NODE_ID_MIN || nodeId > CAN_NODE_ID_MAX)
        return 0U;

    return 1U;
}

static uint8_t CanService_IsAcceptedTarget(const CanService *service, uint8_t targetNodeId)
{
    if (service == NULL)
        return 0U;

    if (targetNodeId == service->localNodeId)
        return 1U;

    if (targetNodeId == CAN_NODE_ID_BROADCAST)
        return 1U;

    return 0U;
}

static uint8_t CanService_IsPrintableAscii(const char *text)
{
    unsigned char ch;

    if (text == NULL || text[0] == '\0')
        return 0U;

    while (*text != '\0')
    {
        ch = (unsigned char)*text;
        if (ch < 32U || ch > 126U)
            return 0U;
        text++;
    }

    return 1U;
}

static uint8_t CanService_NextIndex(uint8_t index, uint8_t capacity)
{
    index++;
    if (index >= capacity)
        index = 0U;
    return index;
}

static uint8_t CanService_CommandQueuePush(CanService *service, const CanMessage *message)
{
    if (service == NULL || message == NULL)
        return 0U;

    if (service->commandCount >= CAN_SERVICE_QUEUE_SIZE)
    {
        service->commandDropCount++;
        service->lastError = CAN_SERVICE_ERROR_TX_QUEUE_FULL;
        return 0U;
    }

    service->commandQueue[service->commandTail] = *message;
    service->commandTail = CanService_NextIndex(service->commandTail, CAN_SERVICE_QUEUE_SIZE);
    service->commandCount++;
    return 1U;
}

static uint8_t CanService_CommandQueuePop(CanService *service, CanMessage *outMessage)
{
    if (service == NULL || outMessage == NULL)
        return 0U;

    if (service->commandCount == 0U)
        return 0U;

    *outMessage = service->commandQueue[service->commandHead];
    CanService_ClearMessage(&service->commandQueue[service->commandHead]);
    service->commandHead = CanService_NextIndex(service->commandHead, CAN_SERVICE_QUEUE_SIZE);
    service->commandCount--;
    return 1U;
}

static uint8_t CanService_EventQueuePush(CanService *service, const CanMessage *message)
{
    if (service == NULL || message == NULL)
        return 0U;

    if (service->eventCount >= CAN_SERVICE_QUEUE_SIZE)
    {
        service->eventDropCount++;
        service->lastError = CAN_SERVICE_ERROR_TX_QUEUE_FULL;
        return 0U;
    }

    service->eventQueue[service->eventTail] = *message;
    service->eventTail = CanService_NextIndex(service->eventTail, CAN_SERVICE_QUEUE_SIZE);
    service->eventCount++;
    return 1U;
}

static uint8_t CanService_EventQueuePop(CanService *service, CanMessage *outMessage)
{
    if (service == NULL || outMessage == NULL)
        return 0U;

    if (service->eventCount == 0U)
        return 0U;

    *outMessage = service->eventQueue[service->eventHead];
    CanService_ClearMessage(&service->eventQueue[service->eventHead]);
    service->eventHead = CanService_NextIndex(service->eventHead, CAN_SERVICE_QUEUE_SIZE);
    service->eventCount--;
    return 1U;
}

static uint8_t CanService_TextQueuePush(CanService *service, const CanMessage *message)
{
    if (service == NULL || message == NULL)
        return 0U;

    if (service->textCount >= CAN_SERVICE_QUEUE_SIZE)
    {
        service->textDropCount++;
        service->lastError = CAN_SERVICE_ERROR_TX_QUEUE_FULL;
        return 0U;
    }

    service->textQueue[service->textTail] = *message;
    service->textTail = CanService_NextIndex(service->textTail, CAN_SERVICE_QUEUE_SIZE);
    service->textCount++;
    return 1U;
}

static uint8_t CanService_TextQueuePop(CanService *service, CanMessage *outMessage)
{
    if (service == NULL || outMessage == NULL)
        return 0U;

    if (service->textCount == 0U)
        return 0U;

    *outMessage = service->textQueue[service->textHead];
    CanService_ClearMessage(&service->textQueue[service->textHead]);
    service->textHead = CanService_NextIndex(service->textHead, CAN_SERVICE_QUEUE_SIZE);
    service->textCount--;
    return 1U;
}

static uint8_t CanService_ResultQueuePush(CanService *service, const CanServiceResult *result)
{
    if (service == NULL || result == NULL)
        return 0U;

    if (service->resultCount >= CAN_SERVICE_QUEUE_SIZE)
    {
        service->lastError = CAN_SERVICE_ERROR_TX_QUEUE_FULL;
        return 0U;
    }

    service->resultQueue[service->resultTail] = *result;
    service->resultTail = CanService_NextIndex(service->resultTail, CAN_SERVICE_QUEUE_SIZE);
    service->resultCount++;
    return 1U;
}

static uint8_t CanService_ResultQueuePop(CanService *service, CanServiceResult *outResult)
{
    if (service == NULL || outResult == NULL)
        return 0U;

    if (service->resultCount == 0U)
        return 0U;

    *outResult = service->resultQueue[service->resultHead];
    CanService_ClearResult(&service->resultQueue[service->resultHead]);
    service->resultHead = CanService_NextIndex(service->resultHead, CAN_SERVICE_QUEUE_SIZE);
    service->resultCount--;
    return 1U;
}

static uint8_t CanService_AllocateRequestId(CanService *service)
{
    uint8_t requestId;

    if (service == NULL)
        return 0U;

    requestId = service->nextRequestId;
    if (requestId == 0U)
        requestId = 1U;

    service->nextRequestId = (uint8_t)(requestId + 1U);
    if (service->nextRequestId == 0U)
        service->nextRequestId = 1U;

    return requestId;
}

static int32_t CanService_FindFreePendingSlot(CanService *service)
{
    uint8_t i;

    if (service == NULL)
        return -1;

    for (i = 0U; i < CAN_SERVICE_PENDING_SIZE; i++)
    {
        if (service->pendingTable[i].inUse == 0U)
            return (int32_t)i;
    }

    return -1;
}

static int32_t CanService_FindPendingByResponse(CanService *service,
                                                uint8_t requestId,
                                                uint8_t sourceNodeId)
{
    uint8_t i;

    if (service == NULL)
        return -1;

    for (i = 0U; i < CAN_SERVICE_PENDING_SIZE; i++)
    {
        if (service->pendingTable[i].inUse == 0U)
            continue;

        if (service->pendingTable[i].requestId != requestId)
            continue;

        if (service->pendingTable[i].targetNodeId != sourceNodeId)
            continue;

        return (int32_t)i;
    }

    return -1;
}

static uint8_t CanService_SendMessage(CanService *service, const CanMessage *message)
{
    CanEncodedFrameList list;

    if (service == NULL || message == NULL)
        return 0U;

    if (CanProto_EncodeMessage(&service->proto, message, &list) == 0U)
    {
        service->sendFailCount++;
        service->lastError = CAN_SERVICE_ERROR_PROTOCOL_ERROR;
        return 0U;
    }

    if (list.count != 1U)
    {
        service->sendFailCount++;
        service->lastError = CAN_SERVICE_ERROR_PROTOCOL_ERROR;
        return 0U;
    }

    if (CanTransport_SendFrame(&service->transport, &list.frames[0]) == 0U)
    {
        service->sendFailCount++;
        service->lastError = CAN_SERVICE_ERROR_TX_QUEUE_FULL;
        return 0U;
    }

    service->sendOkCount++;
    return 1U;
}

static void CanService_FillTimeoutResult(const CanPendingRequest *pending,
                                         uint8_t localNodeId,
                                         CanServiceResult *outResult)
{
    if (pending == NULL || outResult == NULL)
        return;

    CanService_ClearResult(outResult);
    outResult->kind = CAN_SERVICE_RESULT_TIMEOUT;
    outResult->requestId = pending->requestId;
    outResult->sourceNodeId = pending->targetNodeId;
    outResult->targetNodeId = localNodeId;
    outResult->commandCode = pending->commandCode;
    outResult->resultCode = CAN_RES_TIMEOUT;
    outResult->detailCode = 0U;
}

static void CanService_FillResponseResult(const CanPendingRequest *pending,
                                          const CanMessage *message,
                                          CanServiceResult *outResult)
{
    if (pending == NULL || message == NULL || outResult == NULL)
        return;

    CanService_ClearResult(outResult);
    outResult->kind = CAN_SERVICE_RESULT_RESPONSE;
    outResult->requestId = message->requestId;
    outResult->sourceNodeId = message->sourceNodeId;
    outResult->targetNodeId = message->targetNodeId;
    outResult->commandCode = pending->commandCode;
    outResult->resultCode = message->payload[0];
    outResult->detailCode = message->payload[1];
}

static void CanService_ProcessResponse(CanService *service, const CanMessage *message)
{
    int32_t slotIndex;
    CanServiceResult result;

    if (service == NULL || message == NULL)
        return;

    slotIndex = CanService_FindPendingByResponse(service,
                                                 message->requestId,
                                                 message->sourceNodeId);
    if (slotIndex < 0)
    {
        service->responseUnmatchedCount++;
        return;
    }

    CanService_FillResponseResult(&service->pendingTable[slotIndex], message, &result);
    service->pendingTable[slotIndex].inUse = 0U;
    service->responseMatchedCount++;

    if (CanService_ResultQueuePush(service, &result) == 0U)
        service->lastError = CAN_SERVICE_ERROR_TX_QUEUE_FULL;
}

static void CanService_ProcessDecodedMessage(CanService *service, const CanMessage *message)
{
    if (service == NULL || message == NULL)
        return;

    if (CanService_IsAcceptedTarget(service, message->targetNodeId) == 0U)
        return;

    switch (message->messageType)
    {
        case CAN_MSG_COMMAND:
            if (CanService_CommandQueuePush(service, message) != 0U)
                service->commandRxCount++;
            break;

        case CAN_MSG_RESPONSE:
            CanService_ProcessResponse(service, message);
            break;

        case CAN_MSG_EVENT:
            if (CanService_EventQueuePush(service, message) != 0U)
                service->eventRxCount++;
            break;

        case CAN_MSG_TEXT:
            if (CanService_TextQueuePush(service, message) != 0U)
                service->textRxCount++;
            break;

        default:
            service->lastError = CAN_SERVICE_ERROR_UNSUPPORTED;
            break;
    }
}

static void CanService_ProcessRx(CanService *service)
{
    CanFrame frame;
    CanMessage message;
    CanProtoDecodeStatus status;

    if (service == NULL)
        return;

    while (CanTransport_PopRx(&service->transport, &frame) != 0U)
    {
        status = CanProto_DecodeFrame(&service->proto,
                                      &frame,
                                      service->currentTickMs,
                                      &message);
        if (status == CAN_PROTO_DECODE_OK)
            CanService_ProcessDecodedMessage(service, &message);
        else if (status == CAN_PROTO_DECODE_INVALID)
            service->lastError = CAN_SERVICE_ERROR_PROTOCOL_ERROR;
    }
}

static void CanService_ProcessTimeouts(CanService *service)
{
    uint8_t i;
    CanServiceResult result;

    if (service == NULL)
        return;

    for (i = 0U; i < CAN_SERVICE_PENDING_SIZE; i++)
    {
        if (service->pendingTable[i].inUse == 0U)
            continue;

        if ((service->currentTickMs - service->pendingTable[i].startTickMs) <
            service->pendingTable[i].timeoutMs)
            continue;

        CanService_FillTimeoutResult(&service->pendingTable[i],
                                     service->localNodeId,
                                     &result);

        service->pendingTable[i].inUse = 0U;
        service->timeoutCount++;

        if (CanService_ResultQueuePush(service, &result) == 0U)
            service->lastError = CAN_SERVICE_ERROR_TX_QUEUE_FULL;
    }
}

uint8_t CanService_Init(CanService *service, const CanServiceConfig *config)
{
    CanProtoConfig protoConfig;
    CanTransportConfig transportConfig;
    uint8_t i;

    if (service == NULL || config == NULL)
        return 0U;

    (void)memset(service, 0, sizeof(*service));

    service->localNodeId = config->localNodeId;
    service->defaultTimeoutMs = config->defaultTimeoutMs;
    service->nextRequestId = 1U;
    service->lastError = CAN_SERVICE_ERROR_NONE;

    for (i = 0U; i < CAN_SERVICE_PENDING_SIZE; i++)
        CanService_ClearPending(&service->pendingTable[i]);

    protoConfig.localNodeId = config->localNodeId;
    if (CanProto_Init(&service->proto, &protoConfig) == 0U)
    {
        service->lastError = CAN_SERVICE_ERROR_PROTOCOL_ERROR;
        return 0U;
    }

    transportConfig.localNodeId = config->localNodeId;
    transportConfig.instance = config->instance;
    transportConfig.txMbIndex = config->txMbIndex;
    transportConfig.rxMbIndex = config->rxMbIndex;
    transportConfig.driverState = config->driverState;
    transportConfig.userConfig = config->userConfig;
    if (CanTransport_Init(&service->transport, &transportConfig) == 0U)
    {
        service->lastError = CAN_SERVICE_ERROR_NOT_READY;
        return 0U;
    }

    service->initialized = 1U;
    return 1U;
}

void CanService_Task(CanService *service, uint32_t nowMs)
{
    if (service == NULL || service->initialized == 0U)
        return;

    service->currentTickMs = nowMs;

    CanProto_Task(&service->proto, nowMs);
    CanTransport_Task(&service->transport, nowMs);
    CanService_ProcessRx(service);
    CanService_ProcessTimeouts(service);
}

void CanService_FlushTx(CanService *service, uint32_t nowMs)
{
    if (service == NULL || service->initialized == 0U)
        return;

    service->currentTickMs = nowMs;
    CanTransport_Task(&service->transport, nowMs);
}

uint8_t CanService_SendCommand(CanService *service,
                               uint8_t targetNodeId,
                               uint8_t commandCode,
                               uint8_t arg0,
                               uint8_t arg1,
                               uint8_t needResponse)
{
    CanMessage message;
    int32_t slotIndex;
    uint8_t requestId;

    if (service == NULL || service->initialized == 0U)
    {
        if (service != NULL)
            service->lastError = CAN_SERVICE_ERROR_NOT_READY;
        return 0U;
    }

    if (CanService_IsValidTarget(targetNodeId) == 0U)
    {
        service->lastError = CAN_SERVICE_ERROR_INVALID_TARGET;
        return 0U;
    }

    (void)memset(&message, 0, sizeof(message));
    message.version = CAN_PROTO_VERSION_V1;
    message.messageType = CAN_MSG_COMMAND;
    message.sourceNodeId = service->localNodeId;
    message.targetNodeId = targetNodeId;
    message.payloadKind = CAN_PAYLOAD_CTRL_CMD;
    message.payloadLength = 3U;
    message.payload[0] = commandCode;
    message.payload[1] = arg0;
    message.payload[2] = arg1;

    requestId = 0U;
    slotIndex = -1;

    if (needResponse != 0U && targetNodeId != CAN_NODE_ID_BROADCAST)
    {
        slotIndex = CanService_FindFreePendingSlot(service);
        if (slotIndex < 0)
        {
            service->lastError = CAN_SERVICE_ERROR_PENDING_FULL;
            return 0U;
        }

        requestId = CanService_AllocateRequestId(service);
        message.requestId = requestId;
        message.flags |= CAN_MSG_FLAG_NEED_RESPONSE;
    }

    if (CanService_SendMessage(service, &message) == 0U)
        return 0U;

    if (requestId != 0U && slotIndex >= 0)
    {
        service->pendingTable[slotIndex].inUse = 1U;
        service->pendingTable[slotIndex].requestId = requestId;
        service->pendingTable[slotIndex].targetNodeId = targetNodeId;
        service->pendingTable[slotIndex].commandCode = commandCode;
        service->pendingTable[slotIndex].startTickMs = service->currentTickMs;
        service->pendingTable[slotIndex].timeoutMs = service->defaultTimeoutMs;
    }

    return 1U;
}

uint8_t CanService_SendResponse(CanService *service,
                                uint8_t targetNodeId,
                                uint8_t requestId,
                                uint8_t resultCode,
                                uint8_t detailCode)
{
    CanMessage message;

    if (service == NULL || service->initialized == 0U)
        return 0U;

    if (CanService_IsValidTarget(targetNodeId) == 0U || requestId == 0U)
        return 0U;

    (void)memset(&message, 0, sizeof(message));
    message.version = CAN_PROTO_VERSION_V1;
    message.messageType = CAN_MSG_RESPONSE;
    message.sourceNodeId = service->localNodeId;
    message.targetNodeId = targetNodeId;
    message.requestId = requestId;
    message.payloadKind = CAN_PAYLOAD_CTRL_RESULT;
    message.payloadLength = 3U;
    message.payload[0] = resultCode;
    message.payload[1] = detailCode;
    message.payload[2] = 0U;

    return CanService_SendMessage(service, &message);
}

uint8_t CanService_SendEvent(CanService *service,
                             uint8_t targetNodeId,
                             uint8_t eventCode,
                             uint8_t arg0,
                             uint8_t arg1)
{
    CanMessage message;

    if (service == NULL || service->initialized == 0U)
        return 0U;

    if (CanService_IsValidTarget(targetNodeId) == 0U)
        return 0U;

    (void)memset(&message, 0, sizeof(message));
    message.version = CAN_PROTO_VERSION_V1;
    message.messageType = CAN_MSG_EVENT;
    message.sourceNodeId = service->localNodeId;
    message.targetNodeId = targetNodeId;
    message.payloadKind = CAN_PAYLOAD_EVENT_DATA;
    message.payloadLength = 3U;
    message.payload[0] = eventCode;
    message.payload[1] = arg0;
    message.payload[2] = arg1;

    return CanService_SendMessage(service, &message);
}

uint8_t CanService_SendText(CanService *service,
                            uint8_t targetNodeId,
                            uint8_t textType,
                            const char *text)
{
    CanMessage message;
    size_t textLen;

    if (service == NULL || service->initialized == 0U)
    {
        if (service != NULL)
            service->lastError = CAN_SERVICE_ERROR_NOT_READY;
        return 0U;
    }

    if (CanService_IsValidTarget(targetNodeId) == 0U)
    {
        service->lastError = CAN_SERVICE_ERROR_INVALID_TARGET;
        return 0U;
    }

    if (CanService_IsPrintableAscii(text) == 0U)
    {
        service->lastError = CAN_SERVICE_ERROR_PROTOCOL_ERROR;
        return 0U;
    }

    textLen = strlen(text);
    if (textLen == 0U || textLen > CAN_TEXT_MAX_LEN)
    {
        service->lastError = CAN_SERVICE_ERROR_PROTOCOL_ERROR;
        return 0U;
    }

    (void)memset(&message, 0, sizeof(message));
    message.version = CAN_PROTO_VERSION_V1;
    message.messageType = CAN_MSG_TEXT;
    message.sourceNodeId = service->localNodeId;
    message.targetNodeId = targetNodeId;
    message.payloadKind = CAN_PAYLOAD_TEXT_DATA;
    message.textType = textType;
    message.textLength = (uint8_t)textLen;
    (void)memcpy(message.text, text, textLen);
    message.text[textLen] = '\0';

    return CanService_SendMessage(service, &message);
}

uint8_t CanService_PopReceivedCommand(CanService *service, CanMessage *outMessage)
{
    return CanService_CommandQueuePop(service, outMessage);
}

uint8_t CanService_PopResult(CanService *service, CanServiceResult *outResult)
{
    return CanService_ResultQueuePop(service, outResult);
}

uint8_t CanService_PopEvent(CanService *service, CanMessage *outMessage)
{
    return CanService_EventQueuePop(service, outMessage);
}

uint8_t CanService_PopText(CanService *service, CanMessage *outMessage)
{
    return CanService_TextQueuePop(service, outMessage);
}
