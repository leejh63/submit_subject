#ifndef UART_SERVICE_H
#define UART_SERVICE_H

#include "uart_structs.h"

status_t UartService_Init(UartPort *port,
                          uint32_t instance,
                          lpuart_state_t *state,
                          const lpuart_user_config_t *config);

void UartService_OnRxByte(UartPort *port, uint8_t rxByte);

uint8_t UartService_HasLine(const UartPort *port);

status_t UartService_GetLine(UartPort *port,
                             char *outBuffer,
                             uint16_t maxLength);

void UartService_ClearRx(UartPort *port);

status_t UartService_RequestTx(UartPort *port, const char *msg);
status_t UartService_Recover(UartPort *port);

void UartService_ProcessRx(UartPort *port);
void UartService_ProcessTx(UartPort *port);
void UartService_UpdateTx(UartPort *port);
void UartService_ClearTx(UartPort *port);

/* read-only status accessors */
uint8_t UartService_HasError(const UartPort *port);
uint8_t UartService_IsTxBusy(const UartPort *port);

uint16_t UartService_GetCurrentInputLength(const UartPort *port);
status_t UartService_GetCurrentInputText(const UartPort *port,
                                         char *outBuffer,
                                         uint16_t maxLength);

uint16_t UartService_GetTxQueueCount(const UartPort *port);
uint16_t UartService_GetTxQueueCapacity(const UartPort *port);

UartErrorCode UartService_GetErrorCode(const UartPort *port);
#endif
