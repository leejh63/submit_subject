#include "drivers/drv_spi.h"

emb_status_t drv_spi_init(drv_spi_t *drv, const drv_spi_config_t *config)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};

    return drv->config.hal->ops->configure(drv->config.hal, drv->config.hz, drv->config.mode);
}

emb_status_t drv_spi_transfer(drv_spi_t *drv,
                              const uint8_t *txData,
                              uint8_t *rxData,
                              size_t length)
{
    if ((drv == 0) || (length == 0U))
        return EMB_EINVAL;

    return drv->config.hal->ops->transfer(drv->config.hal, txData, rxData, length);
}
