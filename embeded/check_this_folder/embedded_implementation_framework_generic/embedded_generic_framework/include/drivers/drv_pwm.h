#ifndef DRV_PWM_H
#define DRV_PWM_H

#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_pwm.h"

typedef struct
{
    hal_pwm_t *hal;
    uint32_t channel;
    uint32_t periodTicks;
    uint32_t minDutyTicks;
    uint32_t maxDutyTicks;
    uint32_t safeDutyTicks;
} drv_pwm_config_t;

typedef struct
{
    drv_pwm_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
    uint32_t currentDutyTicks;
} drv_pwm_t;

emb_status_t drv_pwm_init(drv_pwm_t *drv, const drv_pwm_config_t *config);
emb_status_t drv_pwm_start(drv_pwm_t *drv);
emb_status_t drv_pwm_stop(drv_pwm_t *drv);
emb_status_t drv_pwm_set_duty(drv_pwm_t *drv, uint32_t dutyTicks);
emb_status_t drv_pwm_set_ratio_permille(drv_pwm_t *drv, uint16_t permille);

#endif
