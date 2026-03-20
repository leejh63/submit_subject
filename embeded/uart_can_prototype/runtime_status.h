#ifndef RUNTIME_STATUS_H
#define RUNTIME_STATUS_H

#include <stdint.h>

/*
 * 런타임 상태 문자열을 만들기 위한 진단 정보 구조체.
 * heartbeat/can/uart 상태를 사람이 보기 쉬운 텍스트로 조립할 때 사용한다.
 */

typedef struct
{
    uint8_t  heartbeatAlive;      /* heartbeat 태스크가 살아 있는지 */
    uint8_t  canAlive;            /* can 태스크가 최근 동작했는지 */
    uint8_t  uartOk;              /* uart가 정상 상태인지 */
    uint32_t tickMs;              /* 현재 소프트 tick */

    uint32_t tmp_check_heartbeat; /* 디버깅용 heartbeat 카운터 */
    uint32_t tmp_check_uart;      /* 디버깅용 uart 카운터 */
    uint32_t tmp_check_can;       /* 디버깅용 can 카운터 */
} RuntimeStatus;

/* RuntimeStatus를 0 기반 상태로 초기화한다. */
void RuntimeStatus_Init(RuntimeStatus *status);

/* heartbeat alive 플래그를 갱신한다. */
void RuntimeStatus_SetHeartbeatAlive(RuntimeStatus *status, uint8_t alive);

/* can alive 플래그를 갱신한다. */
void RuntimeStatus_SetCanAlive(RuntimeStatus *status, uint8_t alive);

/* uart 정상 여부 플래그를 갱신한다. */
void RuntimeStatus_SetUartOk(RuntimeStatus *status, uint8_t ok);

/* 현재 tick 값을 기록한다. */
void RuntimeStatus_SetTickMs(RuntimeStatus *status, uint32_t tickMs);

/* 화면 [TASK] 영역에 들어갈 상태 문자열을 조립한다. */
void RuntimeStatus_BuildTaskText(const RuntimeStatus *status,
                                 char *outBuffer,
                                 uint16_t outBufferSize);

#endif
