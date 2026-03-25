#include "drivers/drv_adc.h"

emb_status_t drv_adc_init(drv_adc_t *drv, const drv_adc_config_t *config)
{
    size_t i;
    emb_status_t status;

    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};
    drv->scanPending = 0U;

    for (i = 0U; i < config->channelCount; ++i)
    {
        status = drv->config.hal->ops->configure_channel(drv->config.hal,
                                                         drv->config.channels[i],
                                                         drv->config.sampleTime);
        if (status != EMB_OK)
            return status;
    }

    return EMB_OK;
}

emb_status_t drv_adc_calibrate(drv_adc_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    return drv->config.hal->ops->calibrate(drv->config.hal);
}

emb_status_t drv_adc_start_scan_dma(drv_adc_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    return drv->config.hal->ops->start_scan_dma(drv->config.hal,
                                                drv->config.channels,
                                                drv->config.channelCount,
                                                drv->config.sampleBuffer);
}

void drv_adc_on_dma_complete_irq(drv_adc_t *drv)
{
    if (drv == 0)
        return;

    drv->scanPending = 1U;
}

void drv_adc_process(drv_adc_t *drv)
{
    if ((drv == 0) || (drv->scanPending == 0U))
        return;

    drv->scanPending = 0U;

    if (drv->config.scanDoneCb != 0)
        drv->config.scanDoneCb(drv->config.context);
}
