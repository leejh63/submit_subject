#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_timer hal_timer_t;

typedef struct
{
    emb_status_t (*start_periodic)(hal_timer_t *hal, uint32_t channel, uint32_t periodTicks);
    emb_status_t (*stop)(hal_timer_t *hal, uint32_t channel);
    emb_status_t (*set_compare)(hal_timer_t *hal, uint32_t channel, uint32_t compareTicks);
} hal_timer_ops_t;

struct hal_timer
{
    const hal_timer_ops_t *ops;
    void *context;
};

#endif
