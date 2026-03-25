#include "drivers/drv_dma.h"

emb_status_t drv_dma_init(drv_dma_t *drv, const drv_dma_config_t *config)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};
    drv->completePending = 0U;

    return EMB_OK;
}

emb_status_t drv_dma_start_memcpy(drv_dma_t *drv, void *dst, const void *src, size_t length)
{
    if ((drv == 0) || (dst == 0) || (src == 0) || (length == 0U))
        return EMB_EINVAL;

    drv->state = EMB_MODULE_STATE_RUNNING;
    return drv->config.hal->ops->start_memcpy(drv->config.hal, dst, src, length);
}

emb_status_t drv_dma_abort(drv_dma_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    return drv->config.hal->ops->abort(drv->config.hal, drv->config.channel);
}

void drv_dma_on_complete_irq(drv_dma_t *drv)
{
    if (drv == 0)
        return;

    drv->completePending = 1U;
}

void drv_dma_process(drv_dma_t *drv)
{
    if ((drv == 0) || (drv->completePending == 0U))
        return;

    drv->completePending = 0U;

    if (drv->config.doneCb != 0)
        drv->config.doneCb(drv->config.context);
}
