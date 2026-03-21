#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_pwm hal_pwm_t;

typedef struct
{
    emb_status_t (*configure)(hal_pwm_t *hal, uint32_t channel, uint32_t periodTicks);
    emb_status_t (*set_duty)(hal_pwm_t *hal, uint32_t channel, uint32_t dutyTicks);
    emb_status_t (*start)(hal_pwm_t *hal, uint32_t channel);
    emb_status_t (*stop)(hal_pwm_t *hal, uint32_t channel);
} hal_pwm_ops_t;

struct hal_pwm
{
    const hal_pwm_ops_t *ops;
    void *context;
};

#endif
