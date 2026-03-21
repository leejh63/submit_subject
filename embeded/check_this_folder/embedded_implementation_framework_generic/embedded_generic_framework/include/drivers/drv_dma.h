#ifndef DRV_DMA_H
#define DRV_DMA_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_dma.h"

typedef void (*drv_dma_done_cb_t)(void *context);

typedef struct
{
    hal_dma_t *hal;
    uint32_t channel;
    drv_dma_done_cb_t doneCb;
    void *context;
} drv_dma_config_t;

typedef struct
{
    drv_dma_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
    volatile uint8_t completePending;
} drv_dma_t;

emb_status_t drv_dma_init(drv_dma_t *drv, const drv_dma_config_t *config);
emb_status_t drv_dma_start_memcpy(drv_dma_t *drv, void *dst, const void *src, size_t length);
emb_status_t drv_dma_abort(drv_dma_t *drv);
void drv_dma_on_complete_irq(drv_dma_t *drv);
void drv_dma_process(drv_dma_t *drv);

#endif
