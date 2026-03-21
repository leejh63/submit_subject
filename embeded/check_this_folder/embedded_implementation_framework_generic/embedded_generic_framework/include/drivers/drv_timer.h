#ifndef DRV_TIMER_H
#define DRV_TIMER_H

#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_timer.h"

typedef void (*drv_timer_cb_t)(void *context);

typedef struct
{
    hal_timer_t *hal;
    uint32_t channel;
    uint32_t periodTicks;
    drv_timer_cb_t callback;
    void *context;
} drv_timer_config_t;

typedef struct
{
    drv_timer_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
    volatile uint32_t tickCount;
    volatile uint8_t pending;
} drv_timer_t;

emb_status_t drv_timer_init(drv_timer_t *drv, const drv_timer_config_t *config);
emb_status_t drv_timer_start(drv_timer_t *drv);
emb_status_t drv_timer_stop(drv_timer_t *drv);
void drv_timer_on_irq(drv_timer_t *drv);
void drv_timer_process(drv_timer_t *drv);

#endif
