#ifndef DRV_UART_H
#define DRV_UART_H

#include <stdint.h>
#include "core/emb_result.h"
#include "hal/hal_s32k_uart.h"

#define DRV_UART_RX_RING_SIZE     128U
#define DRV_UART_TX_QUEUE_DEPTH   4U
#define DRV_UART_TX_CHUNK_SIZE    128U

typedef struct
{
    HalS32kUartPort port;
} DrvUartConfig;

typedef struct
{
    DrvUartConfig config;
    uint8_t started;
    uint8_t txInFlight;
    uint8_t txHead;
    uint8_t txTail;
    uint8_t txCount;
    uint8_t rxHead;
    uint8_t rxTail;
    uint8_t rxCount;
    uint8_t txSize[DRV_UART_TX_QUEUE_DEPTH];
    uint8_t txQueue[DRV_UART_TX_QUEUE_DEPTH][DRV_UART_TX_CHUNK_SIZE];
    uint8_t rxRing[DRV_UART_RX_RING_SIZE];
    uint32_t txStartCount;
    uint32_t txCompleteCount;
    uint32_t txDropCount;
    uint32_t rxByteCount;
    uint32_t rxOverflowCount;
    uint32_t errorCount;
    EmbResult lastError;
} DrvUart;

EmbResult DrvUart_Init(DrvUart *uart, const DrvUartConfig *config);
EmbResult DrvUart_Start(DrvUart *uart);
EmbResult DrvUart_Process(DrvUart *uart);
EmbResult DrvUart_Send(DrvUart *uart, const uint8_t *data, uint32_t size);
EmbResult DrvUart_TryReadByte(DrvUart *uart, uint8_t *outByte);

#endif
