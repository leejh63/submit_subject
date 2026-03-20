#ifndef APP_CTRL_COMMAND_MAILBOX_H
#define APP_CTRL_COMMAND_MAILBOX_H

#include <stdint.h>

#include "app_ctrl_command.h"

/*
 * UART 쪽에서 만든 AppCtrlCommand를 CAN 처리 태스크로 넘기기 위한 고정 크기 큐.
 * 이름은 mailbox지만 실제 구현은 ring buffer다.
 */

#define APP_CTRL_COMMAND_MAILBOX_CAPACITY 4U

typedef struct
{
    AppCtrlCommand storage[APP_CTRL_COMMAND_MAILBOX_CAPACITY]; /* 명령 저장 슬롯 */
    uint8_t head;                           /* pop 위치 */
    uint8_t tail;                           /* push 위치 */
    uint8_t count;                          /* 현재 적재 개수 */
} AppCtrlCommandMailbox;

/* mailbox를 빈 상태로 초기화한다. */
void AppCtrlCommandMailbox_Init(AppCtrlCommandMailbox *mailbox);

/* 처리 대기 중 명령이 하나라도 있는지 확인한다. */
uint8_t AppCtrlCommandMailbox_HasPending(const AppCtrlCommandMailbox *mailbox);

/* mailbox가 가득 찼는지 확인한다. */
uint8_t AppCtrlCommandMailbox_IsFull(const AppCtrlCommandMailbox *mailbox);

/* mailbox가 비어 있는지 확인한다. */
uint8_t AppCtrlCommandMailbox_IsEmpty(const AppCtrlCommandMailbox *mailbox);

/* 현재 적재된 명령 개수를 반환한다. */
uint8_t AppCtrlCommandMailbox_GetCount(const AppCtrlCommandMailbox *mailbox);

/* mailbox 최대 용량을 반환한다. */
uint8_t AppCtrlCommandMailbox_GetCapacity(const AppCtrlCommandMailbox *mailbox);

/* 새 명령을 tail에 적재한다. 실패 시 0을 반환한다. */
uint8_t AppCtrlCommandMailbox_Push(AppCtrlCommandMailbox *mailbox, const AppCtrlCommand *cmd);

/* 가장 오래된 명령을 head에서 꺼낸다. 실패 시 0을 반환한다. */
uint8_t AppCtrlCommandMailbox_Pop(AppCtrlCommandMailbox *mailbox, AppCtrlCommand *outCmd);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (app_ctrl_command_mailbox.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [app_ctrl_command_mailbox.c]
 * - static uint8_t AppCtrlCommandMailbox_NextIndex(uint8_t index);
 *   : ring buffer 인덱스를 다음 위치로 순환시킨다.
 */
#endif
