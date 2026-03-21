#ifndef DRV_GPIO_H
#define DRV_GPIO_H

#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_gpio.h"

typedef struct
{
    hal_gpio_t *hal;
    uint32_t pin;
    uint8_t activeHigh;
} drv_gpio_config_t;

typedef struct
{
    drv_gpio_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
} drv_gpio_t;

emb_status_t drv_gpio_init(drv_gpio_t *drv, const drv_gpio_config_t *config);
emb_status_t drv_gpio_write(drv_gpio_t *drv, uint8_t logicalOn);
emb_status_t drv_gpio_read(drv_gpio_t *drv, uint8_t *logicalOn);

#endif
