#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_flash hal_flash_t;

typedef struct
{
    emb_status_t (*erase)(hal_flash_t *hal, uint32_t address, size_t length);
    emb_status_t (*write)(hal_flash_t *hal, uint32_t address, const void *data, size_t length);
    emb_status_t (*read)(hal_flash_t *hal, uint32_t address, void *data, size_t length);
} hal_flash_ops_t;

struct hal_flash
{
    const hal_flash_ops_t *ops;
    void *context;
};

#endif
