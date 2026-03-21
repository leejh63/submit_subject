#ifndef DRV_I2C_H
#define DRV_I2C_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_i2c.h"

typedef struct
{
    hal_i2c_t *hal;
    uint32_t hz;
    uint8_t retryCount;
} drv_i2c_config_t;

typedef struct
{
    drv_i2c_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
} drv_i2c_t;

emb_status_t drv_i2c_init(drv_i2c_t *drv, const drv_i2c_config_t *config);
emb_status_t drv_i2c_write_read(drv_i2c_t *drv,
                                uint16_t address,
                                const uint8_t *txData,
                                size_t txLength,
                                uint8_t *rxData,
                                size_t rxLength);

#endif
