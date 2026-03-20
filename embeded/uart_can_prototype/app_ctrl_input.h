#ifndef APP_CTRL_INPUT_H
#define APP_CTRL_INPUT_H

#include <stdint.h>

#include "status.h"
#include "app_ctrl_command_mailbox.h"

/*
 * AppUartConsole가 받아온 한 줄 입력을
 * 1) 로컬 명령(help/status 등)인지
 * 2) 원격 제어 명령(open/close/text 등)인지
 * 판별하고, 필요하면 AppCtrlCommandMailbox로 넘기는 모듈의 공개 인터페이스.
 */

#define APP_CTRL_INPUT_TEXT_SIZE 64U  /* 사용자에게 즉시 보여줄 결과 문구 최대 길이 */

/* 입력 처리 후 UART 화면에 즉시 표시할 문구를 담는 구조체 */
typedef struct
{
    char text[APP_CTRL_INPUT_TEXT_SIZE];
} AppCtrlInputResult;

/*
 * 입력 처리 시점의 시스템 스냅샷.
 * AppCtrlInput이 상태 문자열을 만들 때 상위 모듈 내부를 직접 참조하지 않도록 분리해둔 구조다.
 */
typedef struct
{
    uint8_t  consoleState;            /* AppUartConsoleState */
    uint8_t  uartErrorFlag;        /* UART 에러 여부 */
    uint16_t rxLineLength;         /* 현재 입력 길이 */
    uint8_t  txBusy;               /* UART TX 동작 중 여부 */

    uint8_t  commandQueueCount;        /* AppCtrlCommandMailbox 현재 적재 수 */
    uint8_t  commandQueueCapacity;     /* AppCtrlCommandMailbox 최대 적재 수 */

    uint8_t  resultQueueCount;     /* AppCtrlResultBox 현재 적재 수 */
    uint8_t  resultQueueCapacity;  /* AppCtrlResultBox 최대 적재 수 */
} AppCtrlInputSnapshot;

/* 입력 처리 결과 버퍼를 비운다. */
void AppCtrlInputResult_Clear(AppCtrlInputResult *result);

/*
 * 한 줄 입력을 처리한다.
 * - 로컬 명령이면 outResult.text에 즉시 응답을 만든다.
 * - 원격 명령이면 AppCtrlCommand로 파싱 후 mailbox에 push한다.
 * - 결과는 STATUS_SUCCESS / STATUS_BUSY / STATUS_ERROR 로 돌려준다.
 */
status_t AppCtrlInput_HandleLine(const char              *line,
                              const AppCtrlInputSnapshot *snapshot,
                              AppCtrlCommandMailbox             *mailbox,
                              AppCtrlInputResult         *outResult);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (app_ctrl_input.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [app_ctrl_input.c]
 * - static void AppCtrlInput_SetText(AppCtrlInputResult *result, const char *text);
 *   : 결과 문구 버퍼에 안전하게 문자열을 기록한다.
 *
 * - static uint8_t AppCtrlInput_HandleLocalCommand(const char *line,
 *                                               const AppCtrlInputSnapshot *snapshot,
 *                                               AppCtrlInputResult *outResult);
 *   : help / hello / ping / status 같은 로컬 명령을 즉시 처리한다.
 */
#endif
