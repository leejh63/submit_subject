/*
 * 저수준 UART 하드웨어 바인딩 인터페이스다.
 * service 계층은 이 호출들을 사용해 전송을 시작하고,
 * generated driver에서 바이트 단위 callback을 받는다.
 */
#ifndef UART_HW_H
#define UART_HW_H

#include "uart_types.h"

#include "status.h"
#include "lpuart_driver.h"

/*
 * 저수준 UART 하드웨어 진입점 모음이다.
 * service 계층은 이 helper들로 SDK driver를 초기화하고,
 * TX를 시작하며 RX callback을 받는다.
 */
status_t UartHw_InitDefault(UartService *service);
status_t UartHw_StartTransmit(UartService *service, const uint8_t *data, uint16_t length);
status_t UartHw_GetTransmitStatus(UartService *service, uint32_t *bytes_remaining);
void     UartHw_RxCallback(void *driver_state, uart_event_t event, void *user_data);

#endif
