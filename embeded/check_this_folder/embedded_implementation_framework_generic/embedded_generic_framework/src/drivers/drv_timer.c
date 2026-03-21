#include "drivers/drv_timer.h"

emb_status_t drv_timer_init(drv_timer_t *drv, const drv_timer_config_t *config)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};
    drv->tickCount = 0U;
    drv->pending = 0U;
    return EMB_OK;
}

emb_status_t drv_timer_start(drv_timer_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    drv->state = EMB_MODULE_STATE_RUNNING;
    return drv->config.hal->ops->start_periodic(drv->config.hal,
                                                drv->config.channel,
                                                drv->config.periodTicks);
}

emb_status_t drv_timer_stop(drv_timer_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    drv->state = EMB_MODULE_STATE_STOPPED;
    return drv->config.hal->ops->stop(drv->config.hal, drv->config.channel);
}

void drv_timer_on_irq(drv_timer_t *drv)
{
    if (drv == 0)
        return;

    drv->tickCount++;
    drv->pending = 1U;
}

void drv_timer_process(drv_timer_t *drv)
{
    if ((drv == 0) || (drv->pending == 0U))
        return;

    drv->pending = 0U;

    if (drv->config.callback != 0)
        drv->config.callback(drv->config.context);
}
