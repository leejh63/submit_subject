#ifndef CAN_APP_H
#define CAN_APP_H

#include <stdint.h>

#include "can_types.h"
#include "can_service.h"
#include "app_ctrl_command.h"
#include "app_ctrl_result_box.h"

/*
 * CAN 앱 계층의 최상위 모듈.
 * AppCtrlCommand를 받아 CAN service로 내려보내고,
 * service에서 올라온 결과/event/text를 AppCtrlResult로 바꿔 UART 쪽에 전달한다.
 */

#define CAN_APP_RESULT_QUEUE_SIZE   8U

typedef struct
{
    uint8_t     initialized;            /* 초기화 여부 */
    uint8_t     localNodeId;            /* 현재 노드 ID */
    uint8_t     role;                   /* 현재 앱 역할 */
    uint8_t     defaultTargetNodeId;    /* 기본 target */

    CanService  service;                /* 하위 service 계층 */

    AppCtrlResult  resultQueue[CAN_APP_RESULT_QUEUE_SIZE]; /* UART로 넘길 결과 큐 */
    uint8_t     resultHead;
    uint8_t     resultTail;
    uint8_t     resultCount;

    uint32_t    localSubmitOkCount;     /* 로컬 명령 제출 성공 횟수 */
    uint32_t    localSubmitFailCount;   /* 로컬 명령 제출 실패 횟수 */
    uint32_t    remoteCommandCount;     /* 원격 command 처리 횟수 */
    uint32_t    resultConvertCount;     /* service result -> AppCtrlResult 변환 횟수 */
    uint32_t    eventConvertCount;      /* event -> AppCtrlResult 변환 횟수 */
    uint32_t    textConvertCount;       /* text -> AppCtrlResult 변환 횟수 */
    uint32_t    resultDropCount;        /* 결과 큐 포화 드롭 횟수 */
} CanApp;

/* CanApp과 내부 CanService를 초기화한다. */
uint8_t CanApp_Init(CanApp *app,
                    uint8_t localNodeId,
                    uint8_t role,
                    uint8_t defaultTargetNodeId);


// 아래 step API는 can_task 같은 상위 orchestration에서 선택적으로 조합하기 위한 분해형 인터페이스다.
void    CanApp_RunService(CanApp *app, uint32_t nowMs);
void    CanApp_RunRemoteCommands(CanApp *app);
void    CanApp_RunServiceResults(CanApp *app);
void    CanApp_RunEvents(CanApp *app);
void    CanApp_RunTexts(CanApp *app);

/* TX만 한 번 더 밀어주고 싶을 때 사용하는 보조 함수. */
void    CanApp_FlushTx(CanApp *app, uint32_t nowMs);

/* UART에서 파싱한 AppCtrlCommand를 실제 CAN 명령으로 내려보낸다. */
uint8_t CanApp_SubmitAppCtrlCommand(CanApp *app, const AppCtrlCommand *cmd);

/* UART에 보여줄 AppCtrlResult 하나를 앱 결과 큐에서 꺼낸다. */
uint8_t CanApp_PopAppCtrlResult(CanApp *app, AppCtrlResult *outResult);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (can_app.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [can_app.c]
 * - static void CanApp_ClearAppCtrlResult(AppCtrlResult *result);
 *   : AppCtrlResult 슬롯을 기본값으로 초기화한다.
 *
 * - static uint8_t CanApp_ResultNext(uint8_t index);
 *   : 앱 결과 큐의 다음 인덱스를 계산한다.
 *
 * - static uint8_t CanApp_ResultQueueIsFull(const CanApp *app);
 *   : 앱 결과 큐 포화 여부를 검사한다.
 *
 * - static uint8_t CanApp_ResultQueuePush(CanApp *app, const AppCtrlResult *result);
 *   : 변환된 AppCtrlResult를 앱 결과 큐에 적재한다.
 *
 * - static uint8_t CanApp_ResultQueuePop(CanApp *app, AppCtrlResult *outResult);
 *   : UART 쪽으로 넘길 결과를 꺼낸다.
 *
 * - static void CanApp_SetLocalTextResult(CanApp *app, AppCtrlResultType type, const char *text);
 *   : CAN 송신 전 단계에서 발생한 로컬 오류/상태를 결과 큐에 직접 넣는다.
 *
 * - static uint8_t CanApp_IsValidNodeId(uint8_t nodeId);
 *   : target/default target ID 유효성을 검사한다.
 *
 * - static uint8_t CanApp_ResolveTargetNodeId(...);
 *   : AppCtrlCommand의 target 또는 default target 중 실제 전송 대상을 결정한다.
 *
 * - static uint8_t CanApp_MapAppCtrlCommandToCanCommand(...);
 *   : AppCtrlCommandType을 CanCommandCode로 매핑한다.
 *
 * - static AppCtrlResultType CanApp_MapServiceResultType(uint8_t resultCode);
 *   : service result code를 UART용 AppCtrlResultType으로 매핑한다.
 *
 * - static void CanApp_ConvertServiceResult(...);
 *   : CanServiceResult를 UART 표시용 AppCtrlResult로 변환한다.
 *
 * - static void CanApp_ConvertEvent(...);
 *   : 수신 event를 UART 표시 문자열로 변환한다.
 *
 * - static void CanApp_ConvertText(...);
 *   : 수신 text를 UART 표시 문자열로 변환한다.
 *
 * - static void CanApp_HandleRemoteCommand(CanApp *app, const CanMessage *message);
 *   : 원격에서 들어온 command를 현재 노드 로컬 동작으로 처리하고 필요 시 응답한다.
 *
 * - static void CanApp_DrainServiceResults(CanApp *app);
 *   : service result 큐를 모두 꺼내 앱 결과 큐로 변환한다.
 *
 * - static void CanApp_DrainEvents(CanApp *app);
 *   : service event 큐를 모두 꺼내 앱 결과 큐로 변환한다.
 *
 * - static void CanApp_DrainTexts(CanApp *app);
 *   : service text 큐를 모두 꺼내 앱 결과 큐로 변환한다.
 *
 * - static void CanApp_DrainRemoteCommands(CanApp *app);
 *   : service command 큐를 모두 꺼내 로컬 command handler로 보낸다.
 */
#endif
