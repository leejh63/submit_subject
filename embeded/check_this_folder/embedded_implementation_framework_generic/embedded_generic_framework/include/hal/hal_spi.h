#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_spi hal_spi_t;

typedef struct
{
    emb_status_t (*configure)(hal_spi_t *hal, uint32_t hz, uint8_t mode);
    emb_status_t (*transfer)(hal_spi_t *hal,
                             const uint8_t *txData,
                             uint8_t *rxData,
                             size_t length);
} hal_spi_ops_t;

struct hal_spi
{
    const hal_spi_ops_t *ops;
    void *context;
};

#endif
