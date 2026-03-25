#ifndef DRV_WDT_H
#define DRV_WDT_H

#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_wdt.h"

typedef struct
{
    hal_wdt_t *hal;
    uint32_t timeoutMs;
} drv_wdt_config_t;

typedef struct
{
    drv_wdt_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
    uint32_t aliveMask;
    uint32_t requiredMask;
} drv_wdt_t;

emb_status_t drv_wdt_init(drv_wdt_t *drv, const drv_wdt_config_t *config, uint32_t requiredMask);
emb_status_t drv_wdt_start(drv_wdt_t *drv);
void drv_wdt_mark_alive(drv_wdt_t *drv, uint32_t taskBit);
emb_status_t drv_wdt_process(drv_wdt_t *drv);

#endif
