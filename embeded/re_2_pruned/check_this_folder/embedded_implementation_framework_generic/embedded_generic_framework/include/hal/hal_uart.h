#ifndef HAL_UART_H
#define HAL_UART_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_uart hal_uart_t;

typedef struct
{
    emb_status_t (*configure)(hal_uart_t *hal, uint32_t baudrate);
    emb_status_t (*start_rx_irq)(hal_uart_t *hal);
    emb_status_t (*start_tx_irq)(hal_uart_t *hal, const uint8_t *data, size_t length);
    emb_status_t (*start_tx_dma)(hal_uart_t *hal, const uint8_t *data, size_t length);
    emb_status_t (*abort_tx)(hal_uart_t *hal);
} hal_uart_ops_t;

struct hal_uart
{
    const hal_uart_ops_t *ops;
    void *context;
};

#endif
