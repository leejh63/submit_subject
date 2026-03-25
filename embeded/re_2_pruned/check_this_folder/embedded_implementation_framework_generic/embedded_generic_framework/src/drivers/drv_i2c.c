#include "drivers/drv_i2c.h"

emb_status_t drv_i2c_init(drv_i2c_t *drv, const drv_i2c_config_t *config)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};

    return drv->config.hal->ops->configure(drv->config.hal, drv->config.hz);
}

emb_status_t drv_i2c_write_read(drv_i2c_t *drv,
                                uint16_t address,
                                const uint8_t *txData,
                                size_t txLength,
                                uint8_t *rxData,
                                size_t rxLength)
{
    uint8_t tries;
    emb_status_t status = EMB_EIO;

    if (drv == 0)
        return EMB_EINVAL;

    for (tries = 0U; tries <= drv->config.retryCount; ++tries)
    {
        status = drv->config.hal->ops->write_read(drv->config.hal,
                                                  address,
                                                  txData,
                                                  txLength,
                                                  rxData,
                                                  rxLength);
        if (status == EMB_OK)
            return EMB_OK;
    }

    return status;
}
