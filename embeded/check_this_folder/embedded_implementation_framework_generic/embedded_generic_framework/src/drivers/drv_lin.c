#include "drivers/drv_lin.h"

emb_status_t drv_lin_init(drv_lin_t *drv, const drv_lin_config_t *config)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};

    return drv->config.hal->ops->configure(drv->config.hal,
                                           drv->config.baudrate,
                                           (drv->config.role == DRV_LIN_ROLE_MASTER) ? 1U : 0U);
}

emb_status_t drv_lin_send_header(drv_lin_t *drv, uint8_t pid)
{
    if (drv == 0)
        return EMB_EINVAL;

    return drv->config.hal->ops->send_header(drv->config.hal, pid);
}

emb_status_t drv_lin_send_response(drv_lin_t *drv, const uint8_t *data, size_t length)
{
    if ((drv == 0) || (data == 0))
        return EMB_EINVAL;

    return drv->config.hal->ops->send_response(drv->config.hal, data, length);
}

emb_status_t drv_lin_read_response(drv_lin_t *drv, uint8_t *data, size_t length)
{
    if ((drv == 0) || (data == 0))
        return EMB_EINVAL;

    return drv->config.hal->ops->read_response(drv->config.hal, data, length);
}

emb_status_t drv_lin_sleep(drv_lin_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    return drv->config.hal->ops->send_sleep(drv->config.hal);
}

emb_status_t drv_lin_wakeup(drv_lin_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    return drv->config.hal->ops->send_wakeup(drv->config.hal);
}
