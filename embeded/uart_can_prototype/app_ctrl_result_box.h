#ifndef APP_CTRL_RESULT_BOX_H
#define APP_CTRL_RESULT_BOX_H

#include <stdint.h>

/*
 * CAN 처리 결과를 UART 화면 쪽으로 넘기기 위한 고정 크기 결과 큐.
 * mailbox가 "명령 입력" 통로라면 result box는 "처리 결과" 통로다.
 */

#define APP_CTRL_RESULT_TEXT_SIZE 64U
#define APP_CTRL_RESULT_BOX_CAPACITY 4U

typedef enum
{
    APP_CTRL_RESULT_NONE = 0,          /* 초기 상태 */
    APP_CTRL_RESULT_OK,                /* 정상 처리 */
    APP_CTRL_RESULT_ERROR,             /* 일반 오류 */
    APP_CTRL_RESULT_TIMEOUT,           /* 응답 대기 시간 초과 */
    APP_CTRL_RESULT_DENIED,            /* 향후 권한 거부 용도 */
    APP_CTRL_RESULT_INVALID_TARGET     /* 잘못된 targetId */
} AppCtrlResultType;

/* UART에 보여줄 한 건의 결과 데이터 */
typedef struct
{
    AppCtrlResultType type;                /* 결과 종류 */

    uint8_t        hasTargetId;         /* targetId 포함 여부 */
    uint8_t        targetId;            /* 관련 대상 노드 */

    char           text[APP_CTRL_RESULT_TEXT_SIZE]; /* 화면 표시용 문구 */
} AppCtrlResult;

/* AppCtrlResult ring buffer */
typedef struct
{// 큐를 포인터를 쌓게 끔 만드는게 더 좋지않은가?
    AppCtrlResult storage[APP_CTRL_RESULT_BOX_CAPACITY]; /* 결과 저장 슬롯 */
    uint8_t    head;                              /* pop 위치 */
    uint8_t    tail;                              /* push 위치 */
    uint8_t    count;                             /* 현재 적재 개수 */
} AppCtrlResultBox;

/* 단일 결과 구조체를 기본값으로 초기화한다. */
void AppCtrlResult_Clear(AppCtrlResult *result);

/* result box를 빈 상태로 초기화한다. */
void AppCtrlResultBox_Init(AppCtrlResultBox *box);

/* 꺼낼 결과가 남아 있는지 확인한다. */
uint8_t AppCtrlResultBox_HasPending(const AppCtrlResultBox *box);

/* box가 가득 찼는지 확인한다. */
uint8_t AppCtrlResultBox_IsFull(const AppCtrlResultBox *box);

/* box가 비어 있는지 확인한다. */
uint8_t AppCtrlResultBox_IsEmpty(const AppCtrlResultBox *box);

/* 현재 적재된 결과 개수를 반환한다. */
uint8_t AppCtrlResultBox_GetCount(const AppCtrlResultBox *box);

/* box의 최대 용량을 반환한다. */
uint8_t AppCtrlResultBox_GetCapacity(const AppCtrlResultBox *box);

/* 결과를 box에 적재한다. */
uint8_t AppCtrlResultBox_Push(AppCtrlResultBox *box, const AppCtrlResult *result);

/* 가장 오래된 결과를 box에서 꺼낸다. */
uint8_t AppCtrlResultBox_Pop(AppCtrlResultBox *box, AppCtrlResult *outResult);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (app_ctrl_result_box.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [app_ctrl_result_box.c]
 * - static uint8_t AppCtrlResultBox_NextIndex(uint8_t index);
 *   : ring buffer 인덱스를 다음 위치로 순환시킨다.
 */
#endif
