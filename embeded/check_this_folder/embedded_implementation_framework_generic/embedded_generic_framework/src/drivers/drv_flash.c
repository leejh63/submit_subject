#include "drivers/drv_flash.h"

static emb_status_t drv_flash_validate(const drv_flash_t *drv, uint32_t offset, size_t length)
{
    if (drv == 0)
        return EMB_EINVAL;

    if ((offset + length) > drv->config.regionSize)
        return EMB_EINVAL;

    return EMB_OK;
}

emb_status_t drv_flash_init(drv_flash_t *drv, const drv_flash_config_t *config)
{
    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};
    return EMB_OK;
}

emb_status_t drv_flash_read(drv_flash_t *drv, uint32_t offset, void *data, size_t length)
{
    emb_status_t status;

    status = drv_flash_validate(drv, offset, length);
    if (status != EMB_OK)
        return status;

    return drv->config.hal->ops->read(drv->config.hal,
                                      drv->config.baseAddress + offset,
                                      data,
                                      length);
}

emb_status_t drv_flash_write(drv_flash_t *drv, uint32_t offset, const void *data, size_t length)
{
    emb_status_t status;

    status = drv_flash_validate(drv, offset, length);
    if (status != EMB_OK)
        return status;

    return drv->config.hal->ops->write(drv->config.hal,
                                       drv->config.baseAddress + offset,
                                       data,
                                       length);
}

emb_status_t drv_flash_erase(drv_flash_t *drv, uint32_t offset, size_t length)
{
    emb_status_t status;

    status = drv_flash_validate(drv, offset, length);
    if (status != EMB_OK)
        return status;

    return drv->config.hal->ops->erase(drv->config.hal,
                                       drv->config.baseAddress + offset,
                                       length);
}
