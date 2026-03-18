#ifndef UART_HW_H
#define UART_HW_H

#include "uart_structs.h"

status_t UartHw_Init(UartPort *port,
                     uint32_t instance,
                     lpuart_state_t *driverState,
                     const lpuart_user_config_t *userConfig);

status_t UartHw_StartReceiveByte(UartPort *port);
status_t UartHw_ContinueReceiveByte(UartPort *port);

status_t UartHw_StartTransmit(UartPort *port,
                              const uint8_t *data,
                              uint16_t length);

status_t UartHw_GetTransmitStatus(UartPort *port,
                                  uint32_t *bytesRemaining);

void UartHw_RxCallback(void *driverState,
                       uart_event_t event,
                       void *userData);

#endif
