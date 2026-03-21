#ifndef HAL_WDT_H
#define HAL_WDT_H

#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_wdt hal_wdt_t;

typedef struct
{
    emb_status_t (*start)(hal_wdt_t *hal, uint32_t timeoutMs);
    emb_status_t (*kick)(hal_wdt_t *hal);
} hal_wdt_ops_t;

struct hal_wdt
{
    const hal_wdt_ops_t *ops;
    void *context;
};

#endif
