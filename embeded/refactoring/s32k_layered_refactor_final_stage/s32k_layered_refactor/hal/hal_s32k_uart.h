#ifndef HAL_S32K_UART_H
#define HAL_S32K_UART_H

#include <stdint.h>
#include "core/emb_result.h"

typedef enum
{
    HAL_S32K_UART_BIND_NONE = 0,
    HAL_S32K_UART_BIND_MASTER = 1,
    HAL_S32K_UART_BIND_BUTTON = 2
} HalS32kUartBinding;

typedef struct
{
    HalS32kUartBinding binding;
    uint32_t instance;
} HalS32kUartPort;

EmbResult HalS32kUart_Init(HalS32kUartPort *port);
EmbResult HalS32kUart_StartRx(HalS32kUartPort *port, uint8_t *buffer, uint32_t size);
EmbResult HalS32kUart_PollRxByte(HalS32kUartPort *port, uint8_t *outByte);
EmbResult HalS32kUart_IsTxBusy(HalS32kUartPort *port, uint8_t *outBusy);
EmbResult HalS32kUart_Tx(HalS32kUartPort *port, const uint8_t *data, uint32_t size);

#endif
