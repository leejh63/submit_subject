#ifndef UART_HW_H
#define UART_HW_H

#include "uart_shared_types.h"

/*
 * UART HW 레이어 공개 인터페이스.
 * SDK(LPUART_DRV_*)와 직접 맞닿는 부분만 여기 둔다.
 */

/* SDK 드라이버와 callback을 포함하여 UART 하드웨어 수신/송신을 시작 가능한 상태로 만든다. */
status_t UartHw_Init(UartPort *port,
                     uint32_t instance,
                     lpuart_state_t *driverState,
                     const lpuart_user_config_t *userConfig);

/* 1바이트 수신을 새로 시작한다. 주로 초기화 직후 사용한다. */
status_t UartHw_StartReceiveByte(UartPort *port);

/* 다음 1바이트 수신을 다시 걸어준다. callback 이후 연속 수신용이다. */
status_t UartHw_ContinueReceiveByte(UartPort *port);

/* 현재 버퍼를 실제 UART 하드웨어로 송신 시작한다. */
status_t UartHw_StartTransmit(UartPort *port,
                              const uint8_t *data,
                              uint16_t length);

/* 현재 송신 상태와 남은 바이트 수를 SDK에서 조회한다. */
status_t UartHw_GetTransmitStatus(UartPort *port,
                                  uint32_t *bytesRemaining);

/*
 * SDK 수신 callback.
 * 실제 역할은 받은 바이트를 ISR-safe pending buffer에 넣고 다음 수신을 재개하는 것이다.
 */
void UartHw_RxCallback(void *driverState,
                       uart_event_t event,
                       void *userData);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (uart_hw.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [uart_hw.c]
 * - static uint16_t UartHw_RxPending_NextIndex(uint16_t index);
 *   : pending ring buffer 다음 인덱스 계산.
 *
 * - static uint8_t UartHw_RxPending_IsFull(const UartRxPendingBuffer *rxPending);
 *   : ISR이 바이트를 넣기 전에 pending 큐가 꽉 찼는지 확인.
 *
 * - static void UartHw_RxPending_Push(UartPort *port, uint8_t rxByte);
 *   : callback 문맥에서 수신 바이트를 pending 큐에 적재.
 */
#endif
