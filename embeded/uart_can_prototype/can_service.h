#ifndef CAN_SERVICE_H
#define CAN_SERVICE_H

#include <stdint.h>

#include "can_types.h"
#include "can_transport.h"
#include "can_proto.h"
#include "sdk_project_config.h"

/*
 * CAN service 레이어.
 * 이 계층이 실제로 request/response, timeout, 수신 분배(command/event/text)를 담당한다.
 * 즉, transport와 app 사이의 핵심 middle layer다.
 */

/* 응답을 기다리는 outstanding request 1건 */
typedef struct
{
    uint8_t  inUse;         /* 슬롯 사용 중 여부 */
    uint8_t  requestId;     /* 추적용 requestId */
    uint8_t  targetNodeId;  /* 보낸 대상 노드 */
    uint8_t  commandCode;   /* 어떤 명령인지 */
    uint32_t startTickMs;   /* 전송 시각 */
    uint32_t timeoutMs;     /* timeout 기준 */
} CanPendingRequest;

typedef struct
{
    uint8_t  localNodeId;                   /* 현재 노드 ID */
    uint8_t  instance;                      /* FLEXCAN 인스턴스 */
    uint8_t  txMbIndex;                     /* 송신 MB */
    uint8_t  rxMbIndex;                     /* 수신 MB */
    uint32_t defaultTimeoutMs;              /* 명령 기본 timeout */
    flexcan_state_t *driverState;           /* SDK 드라이버 상태 */
    const flexcan_user_config_t *userConfig;/* SDK 설정 */
} CanServiceConfig;

typedef struct
{
    uint8_t initialized;                    /* 초기화 여부 */
    uint8_t localNodeId;                    /* 현재 노드 ID */
    uint8_t nextRequestId;                  /* 다음 requestId 생성기 */
    uint8_t lastError;                      /* 마지막 service 오류 */

    uint32_t defaultTimeoutMs;              /* 기본 timeout */
    uint32_t currentTickMs;                 /* 가장 최근 task tick */

    CanProto     proto;                     /* frame<->message 변환기 */
    CanTransport transport;                 /* frame 송수신 레이어 */

    CanPendingRequest pendingTable[CAN_SERVICE_PENDING_SIZE]; /* 응답 대기 테이블 */

    CanMessage commandQueue[CAN_SERVICE_QUEUE_SIZE]; /* 수신 command 큐 */
    uint8_t    commandHead;
    uint8_t    commandTail;
    uint8_t    commandCount;

    CanServiceResult resultQueue[CAN_SERVICE_QUEUE_SIZE]; /* response/timeout 결과 큐 */
    uint8_t          resultHead;
    uint8_t          resultTail;
    uint8_t          resultCount;

    CanMessage eventQueue[CAN_SERVICE_QUEUE_SIZE]; /* 수신 event 큐 */
    uint8_t    eventHead;
    uint8_t    eventTail;
    uint8_t    eventCount;

    CanMessage textQueue[CAN_SERVICE_QUEUE_SIZE]; /* 수신 text 큐 */
    uint8_t    textHead;
    uint8_t    textTail;
    uint8_t    textCount;

    uint32_t sendOkCount;                   /* 송신 성공 횟수 */
    uint32_t sendFailCount;                 /* 송신 실패 횟수 */
    uint32_t responseMatchedCount;          /* 응답 매칭 성공 횟수 */
    uint32_t responseUnmatchedCount;        /* 매칭 실패 응답 횟수 */
    uint32_t timeoutCount;                  /* timeout 발생 횟수 */
    uint32_t commandRxCount;                /* command 수신 횟수 */
    uint32_t eventRxCount;                  /* event 수신 횟수 */
    uint32_t textRxCount;                   /* text 수신 횟수 */
    uint32_t commandDropCount;              /* command 큐 드롭 횟수 */
    uint32_t eventDropCount;                /* event 큐 드롭 횟수 */
    uint32_t textDropCount;                 /* text 큐 드롭 횟수 */
} CanService;

/* service 계층 전체(proto + transport + pending)를 초기화한다. */
uint8_t CanService_Init(CanService *service, const CanServiceConfig *config);

/* 수신 처리, timeout 처리, transport 진행을 포함한 service 주기 함수. */
void    CanService_Task(CanService *service, uint32_t nowMs);

/* 일반 task 전체 대신 TX 진행만 강제로 한 번 더 밀어주는 보조 함수. */
void    CanService_FlushTx(CanService *service, uint32_t nowMs);

/* command 메시지를 만들어 송신하고, 필요하면 pending table에 등록한다. */
uint8_t CanService_SendCommand(CanService *service,
                               uint8_t targetNodeId,
                               uint8_t commandCode,
                               uint8_t arg0,
                               uint8_t arg1,
                               uint8_t needResponse);

/* response 메시지를 만들어 송신한다. */
uint8_t CanService_SendResponse(CanService *service,
                                uint8_t targetNodeId,
                                uint8_t requestId,
                                uint8_t resultCode,
                                uint8_t detailCode);

/* event 메시지를 만들어 송신한다. */
uint8_t CanService_SendEvent(CanService *service,
                             uint8_t targetNodeId,
                             uint8_t eventCode,
                             uint8_t arg0,
                             uint8_t arg1);

/* 짧은 ASCII text 메시지를 만들어 송신한다. */
uint8_t CanService_SendText(CanService *service,
                            uint8_t targetNodeId,
                            uint8_t textType,
                            const char *text);

/* 수신된 command 하나를 꺼낸다. */
uint8_t CanService_PopReceivedCommand(CanService *service, CanMessage *outMessage);

/* response 또는 timeout 결과 하나를 꺼낸다. */
uint8_t CanService_PopResult(CanService *service, CanServiceResult *outResult);

/* 수신된 event 하나를 꺼낸다. */
uint8_t CanService_PopEvent(CanService *service, CanMessage *outMessage);

/* 수신된 text 하나를 꺼낸다. */
uint8_t CanService_PopText(CanService *service, CanMessage *outMessage);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (can_service.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [can_service.c]
 * - static void CanService_ClearMessage(CanMessage *message);
 *   : CanMessage를 기본값으로 초기화한다.
 *
 * - static void CanService_ClearResult(CanServiceResult *result);
 *   : CanServiceResult를 기본값으로 초기화한다.
 *
 * - static void CanService_ClearPending(CanPendingRequest *pending);
 *   : pending 슬롯을 비운다.
 *
 * - static uint8_t CanService_IsValidTarget(uint8_t nodeId);
 *   : 전송 대상 ID가 유효한지 검사한다.
 *
 * - static uint8_t CanService_IsAcceptedTarget(const CanService *service, uint8_t targetNodeId);
 *   : 현재 노드가 이 target 메시지를 받아야 하는지 검사한다.
 *
 * - static uint8_t CanService_IsPrintableAscii(const char *text);
 *   : text 송신 시 본문이 출력 가능한 ASCII인지 검사한다.
 *
 * - static uint8_t CanService_NextIndex(uint8_t index, uint8_t capacity);
 *   : 공통 ring buffer 다음 인덱스 계산.
 *
 * - static uint8_t CanService_CommandQueuePush(...), CommandQueuePop(...);
 *   : 수신 command 큐 적재/소비.
 *
 * - static uint8_t CanService_EventQueuePush(...), EventQueuePop(...);
 *   : 수신 event 큐 적재/소비.
 *
 * - static uint8_t CanService_TextQueuePush(...), TextQueuePop(...);
 *   : 수신 text 큐 적재/소비.
 *
 * - static uint8_t CanService_ResultQueuePush(...), ResultQueuePop(...);
 *   : response/timeout 결과 큐 적재/소비.
 *
 * - static uint8_t CanService_AllocateRequestId(CanService *service);
 *   : 다음 requestId를 생성한다.
 *
 * - static int32_t CanService_FindFreePendingSlot(CanService *service);
 *   : 빈 pending 슬롯을 찾는다.
 *
 * - static int32_t CanService_FindPendingByResponse(CanService *service, const CanMessage *message);
 *   : 들어온 response가 어떤 pending과 매칭되는지 찾는다.
 *
 * - static uint8_t CanService_SendMessage(CanService *service, const CanMessage *message);
 *   : proto 인코딩 후 transport 송신까지 한 번에 수행한다.
 *
 * - static void CanService_FillTimeoutResult(...);
 *   : timeout 발생 시 상위에 올릴 결과 구조를 채운다.
 *
 * - static void CanService_FillResponseResult(...);
 *   : 정상 response를 상위 결과 구조로 변환한다.
 *
 * - static void CanService_ProcessResponse(CanService *service, const CanMessage *message);
 *   : 들어온 response를 pending과 매칭하고 resultQueue로 변환한다.
 *
 * - static void CanService_ProcessDecodedMessage(CanService *service, const CanMessage *message);
 *   : decode된 메시지를 command/event/text/response 흐름으로 분기한다.
 *
 * - static void CanService_ProcessRx(CanService *service);
 *   : transport RX 큐를 반복 소비하며 decode 및 분배를 수행한다.
 *
 * - static void CanService_ProcessTimeouts(CanService *service);
 *   : pending table을 순회하며 timeout을 감지한다.
 */
#endif
