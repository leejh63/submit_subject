#include "drivers/drv_pwm.h"

static uint32_t drv_pwm_clamp(const drv_pwm_t *drv, uint32_t dutyTicks)
{
    if (dutyTicks < drv->config.minDutyTicks)
        return drv->config.minDutyTicks;
    if (dutyTicks > drv->config.maxDutyTicks)
        return drv->config.maxDutyTicks;
    return dutyTicks;
}

emb_status_t drv_pwm_init(drv_pwm_t *drv, const drv_pwm_config_t *config)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};
    drv->currentDutyTicks = config->safeDutyTicks;

    return drv->config.hal->ops->configure(drv->config.hal,
                                           drv->config.channel,
                                           drv->config.periodTicks);
}

emb_status_t drv_pwm_start(drv_pwm_t *drv)
{
    emb_status_t status;

    if (drv == 0)
        return EMB_EINVAL;

    status = drv_pwm_set_duty(drv, drv->config.safeDutyTicks);
    if (status != EMB_OK)
        return status;

    drv->state = EMB_MODULE_STATE_RUNNING;
    return drv->config.hal->ops->start(drv->config.hal, drv->config.channel);
}

emb_status_t drv_pwm_stop(drv_pwm_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    drv->state = EMB_MODULE_STATE_STOPPED;
    return drv->config.hal->ops->stop(drv->config.hal, drv->config.channel);
}

emb_status_t drv_pwm_set_duty(drv_pwm_t *drv, uint32_t dutyTicks)
{
    if (drv == 0)
        return EMB_EINVAL;

    dutyTicks = drv_pwm_clamp(drv, dutyTicks);
    drv->currentDutyTicks = dutyTicks;
    return drv->config.hal->ops->set_duty(drv->config.hal, drv->config.channel, dutyTicks);
}

emb_status_t drv_pwm_set_ratio_permille(drv_pwm_t *drv, uint16_t permille)
{
    uint64_t scaled;
    uint32_t dutyTicks;

    if (drv == 0)
        return EMB_EINVAL;
    if (permille > 1000U)
        permille = 1000U;

    scaled = ((uint64_t)drv->config.periodTicks * (uint64_t)permille) / 1000ULL;
    dutyTicks = (uint32_t)scaled;
    return drv_pwm_set_duty(drv, dutyTicks);
}
