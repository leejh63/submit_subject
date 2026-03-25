#include "drivers/drv_gpio.h"

emb_status_t drv_gpio_init(drv_gpio_t *drv, const drv_gpio_config_t *config)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};

    return drv->config.hal->ops->configure_output(drv->config.hal,
                                                  drv->config.pin,
                                                  drv->config.activeHigh,
                                                  0U);
}

emb_status_t drv_gpio_write(drv_gpio_t *drv, uint8_t logicalOn)
{
    uint8_t physicalLevel;

    if ((drv == 0) || (drv->state == EMB_MODULE_STATE_RESET))
        return EMB_ESTATE;

    physicalLevel = drv->config.activeHigh ? logicalOn : (uint8_t)!logicalOn;
    return drv->config.hal->ops->write(drv->config.hal, drv->config.pin, physicalLevel);
}

emb_status_t drv_gpio_read(drv_gpio_t *drv, uint8_t *logicalOn)
{
    uint8_t physicalLevel;
    emb_status_t status;

    if ((drv == 0) || (logicalOn == 0))
        return EMB_EINVAL;

    status = drv->config.hal->ops->read(drv->config.hal, drv->config.pin, &physicalLevel);
    if (status != EMB_OK)
        return status;

    *logicalOn = drv->config.activeHigh ? physicalLevel : (uint8_t)!physicalLevel;
    return EMB_OK;
}
