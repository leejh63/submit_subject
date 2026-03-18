#ifndef CAN_SERVICE_H
#define CAN_SERVICE_H

#include <stdint.h>

#include "can_types.h"
#include "can_transport.h"
#include "can_proto.h"
#include "sdk_project_config.h"

typedef struct
{
    uint8_t  inUse;
    uint8_t  requestId;
    uint8_t  targetNodeId;
    uint8_t  commandCode;
    uint32_t startTickMs;
    uint32_t timeoutMs;
} CanPendingRequest;

typedef struct
{
    uint8_t  localNodeId;
    uint8_t  instance;
    uint8_t  txMbIndex;
    uint8_t  rxMbIndex;
    uint32_t defaultTimeoutMs;
    flexcan_state_t *driverState;
    const flexcan_user_config_t *userConfig;
} CanServiceConfig;

typedef struct
{
    uint8_t initialized;
    uint8_t localNodeId;
    uint8_t nextRequestId;
    uint8_t lastError;

    uint32_t defaultTimeoutMs;
    uint32_t currentTickMs;

    CanProto     proto;
    CanTransport transport;

    CanPendingRequest pendingTable[CAN_SERVICE_PENDING_SIZE];

    CanMessage commandQueue[CAN_SERVICE_QUEUE_SIZE];
    uint8_t    commandHead;
    uint8_t    commandTail;
    uint8_t    commandCount;

    CanServiceResult resultQueue[CAN_SERVICE_QUEUE_SIZE];
    uint8_t          resultHead;
    uint8_t          resultTail;
    uint8_t          resultCount;

    CanMessage eventQueue[CAN_SERVICE_QUEUE_SIZE];
    uint8_t    eventHead;
    uint8_t    eventTail;
    uint8_t    eventCount;

    CanMessage textQueue[CAN_SERVICE_QUEUE_SIZE];
    uint8_t    textHead;
    uint8_t    textTail;
    uint8_t    textCount;

    uint32_t sendOkCount;
    uint32_t sendFailCount;
    uint32_t responseMatchedCount;
    uint32_t responseUnmatchedCount;
    uint32_t timeoutCount;
    uint32_t commandRxCount;
    uint32_t eventRxCount;
    uint32_t textRxCount;
    uint32_t commandDropCount;
    uint32_t eventDropCount;
    uint32_t textDropCount;
} CanService;

uint8_t CanService_Init(CanService *service, const CanServiceConfig *config);
void    CanService_Task(CanService *service, uint32_t nowMs);
void    CanService_FlushTx(CanService *service, uint32_t nowMs);

uint8_t CanService_SendCommand(CanService *service,
                               uint8_t targetNodeId,
                               uint8_t commandCode,
                               uint8_t arg0,
                               uint8_t arg1,
                               uint8_t needResponse);

uint8_t CanService_SendResponse(CanService *service,
                                uint8_t targetNodeId,
                                uint8_t requestId,
                                uint8_t resultCode,
                                uint8_t detailCode);

uint8_t CanService_SendEvent(CanService *service,
                             uint8_t targetNodeId,
                             uint8_t eventCode,
                             uint8_t arg0,
                             uint8_t arg1);

uint8_t CanService_SendText(CanService *service,
                            uint8_t targetNodeId,
                            uint8_t textType,
                            const char *text);

uint8_t CanService_PopReceivedCommand(CanService *service, CanMessage *outMessage);
uint8_t CanService_PopResult(CanService *service, CanServiceResult *outResult);
uint8_t CanService_PopEvent(CanService *service, CanMessage *outMessage);
uint8_t CanService_PopText(CanService *service, CanMessage *outMessage);

#endif
