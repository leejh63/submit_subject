#ifndef CAN_TRANSPORT_H
#define CAN_TRANSPORT_H

#include <stdint.h>

#include "can_types.h"
#include "can_hw.h"
#include "sdk_project_config.h"

/*
 * CAN transport 레이어.
 * 역할은 단순하다.
 * - 상위에서 내려온 CanFrame을 TX 큐에 저장
 * - HW에서 올라온 CanFrame을 RX 큐에 저장
 * - HW busy 상태와 큐 사이를 중재
 */

typedef struct
{
    uint8_t localNodeId;                   /* 현재 노드 ID */
    uint8_t instance;                      /* FLEXCAN 인스턴스 */
    uint8_t txMbIndex;                     /* 송신 MB */
    uint8_t rxMbIndex;                     /* 수신 MB */
    flexcan_state_t *driverState;          /* SDK 드라이버 상태 */
    const flexcan_user_config_t *userConfig; /* SDK 설정 */
} CanTransportConfig;

typedef struct
{
    uint8_t initialized;                   /* 초기화 여부 */
    uint8_t txInFlight;                    /* 현재 currentTxFrame이 HW로 나갔는지 */
    uint8_t lastError;                     /* 마지막 transport 오류 */

    CanHw   hw;                            /* 실제 HW 레이어 */

    CanFrame txQueue[CAN_TRANSPORT_TX_QUEUE_SIZE]; /* 송신 대기 큐 */
    uint8_t  txHead;
    uint8_t  txTail;
    uint8_t  txCount;

    CanFrame rxQueue[CAN_TRANSPORT_RX_QUEUE_SIZE]; /* 수신 전달 큐 */
    uint8_t  rxHead;
    uint8_t  rxTail;
    uint8_t  rxCount;

    CanFrame currentTxFrame;               /* 현재 HW로 내보내는 프레임 */
    uint32_t hwTxOkCountSeen;              /* transport가 반영한 HW TX 성공 횟수 */
    uint32_t hwTxErrorCountSeen;           /* transport가 반영한 HW TX 실패 횟수 */

    uint32_t txQueuedCount;                /* TX 큐 적재 횟수 */
    uint32_t txStartCount;                 /* HW 송신 시작 횟수 */
    uint32_t txRetryCount;                 /* HW busy로 재시도한 횟수 */
    uint32_t txDropCount;                  /* TX 큐 포화 드롭 횟수 */

    uint32_t rxQueuedCount;                /* RX 큐 적재 횟수 */
    uint32_t rxDropCount;                  /* RX 큐 포화 드롭 횟수 */
} CanTransport;

/* transport와 내부 hw 레이어를 초기화한다. */
uint8_t CanTransport_Init(CanTransport *transport,
                          const CanTransportConfig *config);

/* HW <-> transport 큐 사이의 이동을 처리한다. */
void    CanTransport_Task(CanTransport *transport, uint32_t nowMs);

/* 상위에서 받은 프레임을 TX 큐에 적재한다. */
uint8_t CanTransport_SendFrame(CanTransport *transport,
                               const CanFrame *frame);

/* transport RX 큐에서 프레임 하나를 꺼낸다. */
uint8_t CanTransport_PopRx(CanTransport *transport,
                           CanFrame *outFrame);

/* transport가 사용 가능한 상태인지 확인한다. */
uint8_t CanTransport_IsReady(const CanTransport *transport);

/* 마지막 transport 오류 코드를 조회한다. */
uint8_t CanTransport_GetLastError(const CanTransport *transport);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (can_transport.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [can_transport.c]
 * - static void CanTransport_ClearFrame(CanFrame *frame);
 *   : CanFrame을 기본값으로 초기화한다.
 *
 * - static uint8_t CanTransport_TxNext(uint8_t index);
 *   : TX ring buffer 다음 인덱스.
 *
 * - static uint8_t CanTransport_RxNext(uint8_t index);
 *   : RX ring buffer 다음 인덱스.
 *
 * - static uint8_t CanTransport_TxIsFull(const CanTransport *transport);
 *   : TX 큐 포화 여부 검사.
 *
 * - static uint8_t CanTransport_RxIsFull(const CanTransport *transport);
 *   : RX 큐 포화 여부 검사.
 *
 * - static uint8_t CanTransport_TxPeek(const CanTransport *transport, CanFrame *outFrame);
 *   : TX 큐 front를 제거하지 않고 확인한다.
 *
 * - static void CanTransport_TxDropFront(CanTransport *transport);
 *   : TX 완료가 확인된 뒤 TX 큐 front를 실제로 소비한다.
 *
 * - static uint8_t CanTransport_RxPush(CanTransport *transport, const CanFrame *frame);
 *   : HW에서 올라온 프레임을 transport RX 큐에 적재한다.
 *
 * - static void CanTransport_DrainHwRx(CanTransport *transport);
 *   : HW RX 큐에 있는 프레임을 transport RX 큐로 옮긴다.
 *
 * - static void CanTransport_ProcessTx(CanTransport *transport);
 *   : HW가 비었으면 TX 큐 front를 실제 송신 시작한다.
 */
#endif
