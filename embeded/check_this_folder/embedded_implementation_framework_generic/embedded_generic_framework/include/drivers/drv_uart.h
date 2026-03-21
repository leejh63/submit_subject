#ifndef DRV_UART_H
#define DRV_UART_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "core/emb_time.h"
#include "hal/hal_uart.h"
#include "infra/emb_ring_u8.h"

typedef void (*drv_uart_rx_ready_cb_t)(void *context);

typedef struct
{
    hal_uart_t *hal;
    uint32_t baudrate;
    uint8_t *rxStorage;
    size_t rxCapacity;
    drv_uart_rx_ready_cb_t rxReadyCb;
    void *context;
} drv_uart_config_t;

typedef struct
{
    drv_uart_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
    emb_ring_u8_t rxRing;
    volatile uint8_t rxPending;
    volatile uint8_t txBusy;
} drv_uart_t;

emb_status_t drv_uart_init(drv_uart_t *drv, const drv_uart_config_t *config);
emb_status_t drv_uart_start(drv_uart_t *drv);
emb_status_t drv_uart_send_irq(drv_uart_t *drv, const uint8_t *data, size_t length);
emb_status_t drv_uart_send_dma(drv_uart_t *drv, const uint8_t *data, size_t length);
emb_status_t drv_uart_read_byte(drv_uart_t *drv, uint8_t *outByte);
void drv_uart_on_rx_byte_irq(drv_uart_t *drv, uint8_t byte);
void drv_uart_on_tx_done_irq(drv_uart_t *drv);
void drv_uart_process(drv_uart_t *drv);

#endif
