#ifndef SVC_UART_CONSOLE_H
#define SVC_UART_CONSOLE_H

#include <stdint.h>
#include "core/emb_result.h"
#include "drivers/drv_uart.h"

#define SVC_UART_CONSOLE_LINE_MAX  64U
#define SVC_UART_CONSOLE_TX_MAX    128U

typedef struct
{
    const char *prompt;
    const char *banner;
} SvcUartConsoleConfig;

typedef struct
{
    uint32_t nowMs;
    uint32_t buttonEventCount;
    uint32_t linPollIssueCount;
    uint32_t linAckIssueCount;
    uint32_t linFaultCount;
    uint32_t uartRxOverflowCount;
    uint32_t uartTxDropCount;
    uint32_t gatewaySensorTimeoutCount;
    uint32_t gatewayAckRequestCount;
    uint32_t gatewayAckSendCount;
    uint32_t gatewayAckConfirmCount;
    uint32_t gatewayAckGiveUpCount;
    uint8_t linStatusValid;
    uint16_t linAdcRaw;
    uint8_t linZone;
    uint8_t linEmergencyLatched;
    uint8_t lastButtonId;
    uint8_t lastButtonAction;
    uint8_t lastButtonSequence;
} SvcUartConsoleSnapshot;

typedef struct
{
    SvcUartConsoleConfig config;
    char lineBuffer[SVC_UART_CONSOLE_LINE_MAX];
    uint8_t lineLength;
    uint8_t promptPending;
    uint8_t ackRequestPending;
    uint8_t pollRequestPending;
    uint32_t lineCount;
    uint32_t parseErrorCount;
    uint32_t unknownCommandCount;
} SvcUartConsole;

EmbResult SvcUartConsole_Init(SvcUartConsole *console,
                              const SvcUartConsoleConfig *config);
EmbResult SvcUartConsole_Start(SvcUartConsole *console, DrvUart *uart);
void SvcUartConsole_Process(SvcUartConsole *console,
                            DrvUart *uart,
                            const SvcUartConsoleSnapshot *snapshot);
uint8_t SvcUartConsole_TakeAckRequest(SvcUartConsole *console);
uint8_t SvcUartConsole_TakePollRequest(SvcUartConsole *console);

#endif
