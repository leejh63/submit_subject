#ifndef DRV_FLASH_H
#define DRV_FLASH_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_flash.h"

typedef struct
{
    hal_flash_t *hal;
    uint32_t baseAddress;
    size_t regionSize;
} drv_flash_config_t;

typedef struct
{
    drv_flash_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
} drv_flash_t;

emb_status_t drv_flash_init(drv_flash_t *drv, const drv_flash_config_t *config);
emb_status_t drv_flash_read(drv_flash_t *drv, uint32_t offset, void *data, size_t length);
emb_status_t drv_flash_write(drv_flash_t *drv, uint32_t offset, const void *data, size_t length);
emb_status_t drv_flash_erase(drv_flash_t *drv, uint32_t offset, size_t length);

#endif
