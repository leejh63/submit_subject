#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_i2c hal_i2c_t;

typedef struct
{
    emb_status_t (*configure)(hal_i2c_t *hal, uint32_t hz);
    emb_status_t (*write)(hal_i2c_t *hal, uint16_t address, const uint8_t *data, size_t length);
    emb_status_t (*read)(hal_i2c_t *hal, uint16_t address, uint8_t *data, size_t length);
    emb_status_t (*write_read)(hal_i2c_t *hal,
                               uint16_t address,
                               const uint8_t *txData,
                               size_t txLength,
                               uint8_t *rxData,
                               size_t rxLength);
    emb_status_t (*recover_bus)(hal_i2c_t *hal);
} hal_i2c_ops_t;

struct hal_i2c
{
    const hal_i2c_ops_t *ops;
    void *context;
};

#endif
