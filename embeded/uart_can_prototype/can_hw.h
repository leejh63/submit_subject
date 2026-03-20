#ifndef CAN_HW_H
#define CAN_HW_H

#include <stdint.h>

#include "can_types.h"
#include "sdk_project_config.h"

/*
 * CAN 하드웨어 직접 제어 레이어.
 * FLEXCAN 드라이버 초기화, MB 설정, HW TX 시작, HW RX polling을 담당한다.
 */

typedef struct
{
    uint8_t localNodeId;                   /* 현재 노드 ID */
    uint8_t instance;                      /* FLEXCAN 인스턴스 */
    uint8_t txMbIndex;                     /* 송신 MB 인덱스 */
    uint8_t rxMbIndex;                     /* 수신 MB 인덱스 */
    flexcan_state_t *driverState;          /* SDK 드라이버 상태 객체 */
    const flexcan_user_config_t *userConfig; /* SDK 초기화 설정 */
} CanHwConfig;

typedef struct
{
    uint8_t initialized;                   /* 초기화 완료 여부 */
    uint8_t localNodeId;                   /* 현재 노드 ID */
    uint8_t instance;                      /* FLEXCAN 인스턴스 */
    uint8_t txMbIndex;                     /* 송신 MB 인덱스 */
    uint8_t rxMbIndex;                     /* 수신 MB 인덱스 */

    uint8_t txBusy;                        /* HW 송신 진행 여부 */
    uint8_t lastError;                     /* 마지막 하드웨어 오류 */

    flexcan_state_t *driverState;          /* SDK 드라이버 상태 객체 */
    const flexcan_user_config_t *userConfig; /* SDK 초기화 설정 */

    flexcan_msgbuff_t rxMsg;               /* SDK 수신용 임시 버퍼 */

    CanFrame rxQueue[CAN_HW_RX_QUEUE_SIZE];/* HW에서 읽어온 프레임 큐 */
    uint8_t  rxHead;                       /* pop 위치 */
    uint8_t  rxTail;                       /* push 위치 */
    uint8_t  rxCount;                      /* 적재 개수 */

    uint32_t txOkCount;                    /* TX 성공 횟수 */
    uint32_t txErrorCount;                 /* TX 실패 횟수 */
    uint32_t rxOkCount;                    /* RX 성공 횟수 */
    uint32_t rxErrorCount;                 /* RX 실패 횟수 */
    uint32_t rxDropCount;                  /* RX 큐 포화로 드롭된 횟수 */
} CanHw;

/* FLEXCAN 드라이버, MB, RX 재시작 상태를 초기화한다. */
uint8_t CanHw_Init(CanHw *hw, const CanHwConfig *config);

/* HW RX polling과 TX 완료 감시를 수행한다. */
void    CanHw_Task(CanHw *hw, uint32_t nowMs);

/* 원시 프레임 하나의 실제 CAN 송신을 시작한다. */
uint8_t CanHw_StartTx(CanHw *hw, const CanFrame *frame);

/* HW 수신 큐에서 프레임 하나를 꺼낸다. */
uint8_t CanHw_TryPopRx(CanHw *hw, CanFrame *outFrame);

/* HW 레이어가 초기화되어 사용 가능한지 확인한다. */
uint8_t CanHw_IsReady(const CanHw *hw);

/* HW TX가 아직 끝나지 않았는지 확인한다. */
uint8_t CanHw_IsTxBusy(const CanHw *hw);

/* 마지막 하드웨어 오류 코드를 반환한다. */
uint8_t CanHw_GetLastError(const CanHw *hw);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (can_hw.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [can_hw.c]
 * - static void CanHw_ClearFrame(CanFrame *frame);
 *   : CanFrame을 기본값으로 초기화한다.
 *
 * - static uint8_t CanHw_RxQueueNext(uint8_t index);
 *   : RX ring buffer 다음 인덱스를 계산한다.
 *
 * - static uint8_t CanHw_RxQueueIsFull(const CanHw *hw);
 *   : HW RX 큐 포화 여부를 검사한다.
 *
 * - static uint8_t CanHw_RxQueuePush(CanHw *hw, const CanFrame *frame);
 *   : 수신된 프레임을 HW RX 큐에 적재한다.
 *
 * - static void CanHw_CopySdkRxToFrame(const flexcan_msgbuff_t *rxMsg,
 *                                      uint32_t nowMs,
 *                                      CanFrame *outFrame);
 *   : SDK msg buffer를 공통 CanFrame 형식으로 복사한다.
 *
 * - static void CanHw_FillDataInfo(const CanFrame *frame, flexcan_data_info_t *outInfo);
 *   : SDK 송신에 필요한 data info 구조를 채운다.
 *
 * - static uint8_t CanHw_StartReceive(CanHw *hw);
 *   : 현재 RX MB에 다음 수신을 다시 걸어둔다.
 *
 * - static uint8_t CanHw_ConfigAcceptAll(CanHw *hw);
 *   : 현재 구현에서 모든 표준 ID를 받도록 RX 필터를 설정한다.
 */
#endif
