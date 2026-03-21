#ifndef DRV_SPI_H
#define DRV_SPI_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_spi.h"

typedef struct
{
    hal_spi_t *hal;
    uint32_t hz;
    uint8_t mode;
} drv_spi_config_t;

typedef struct
{
    drv_spi_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
} drv_spi_t;

emb_status_t drv_spi_init(drv_spi_t *drv, const drv_spi_config_t *config);
emb_status_t drv_spi_transfer(drv_spi_t *drv,
                              const uint8_t *txData,
                              uint8_t *rxData,
                              size_t length);

#endif
