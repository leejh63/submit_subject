#include "drivers/drv_wdt.h"

emb_status_t drv_wdt_init(drv_wdt_t *drv, const drv_wdt_config_t *config, uint32_t requiredMask)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};
    drv->aliveMask = 0U;
    drv->requiredMask = requiredMask;
    return EMB_OK;
}

emb_status_t drv_wdt_start(drv_wdt_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    drv->state = EMB_MODULE_STATE_RUNNING;
    return drv->config.hal->ops->start(drv->config.hal, drv->config.timeoutMs);
}

void drv_wdt_mark_alive(drv_wdt_t *drv, uint32_t taskBit)
{
    if (drv == 0)
        return;

    drv->aliveMask |= taskBit;
}

emb_status_t drv_wdt_process(drv_wdt_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    if ((drv->aliveMask & drv->requiredMask) != drv->requiredMask)
        return EMB_ESTATE;

    drv->aliveMask = 0U;
    return drv->config.hal->ops->kick(drv->config.hal);
}
