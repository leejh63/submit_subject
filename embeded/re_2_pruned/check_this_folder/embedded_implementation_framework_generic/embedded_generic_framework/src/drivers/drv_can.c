#include "drivers/drv_can.h"

emb_status_t drv_can_init(drv_can_t *drv, const drv_can_config_t *config)
{
    emb_status_t status;

    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};
    drv->rxPending = 0U;

    status = emb_msg_queue_init(&drv->rxQueue,
                                config->rxQueueStorage,
                                sizeof(hal_can_frame_t),
                                config->rxQueueCapacity);
    if (status != EMB_OK)
        return status;

    status = drv->config.hal->ops->configure(drv->config.hal, drv->config.bitrate);
    if (status != EMB_OK)
        return status;

    return EMB_OK;
}

emb_status_t drv_can_start(drv_can_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    drv->state = EMB_MODULE_STATE_RUNNING;
    return drv->config.hal->ops->start(drv->config.hal);
}

emb_status_t drv_can_send(drv_can_t *drv, const hal_can_frame_t *frame)
{
    if ((drv == 0) || (frame == 0))
        return EMB_EINVAL;

    return drv->config.hal->ops->send(drv->config.hal, frame);
}

emb_status_t drv_can_receive(drv_can_t *drv, hal_can_frame_t *outFrame)
{
    if ((drv == 0) || (outFrame == 0))
        return EMB_EINVAL;

    return emb_msg_queue_pop(&drv->rxQueue, outFrame);
}

void drv_can_on_rx_irq(drv_can_t *drv, const hal_can_frame_t *frame)
{
    if ((drv == 0) || (frame == 0))
        return;

    if (emb_msg_queue_push(&drv->rxQueue, frame) != EMB_OK)
        drv->stats.dropCount++;

    drv->rxPending = 1U;
}

void drv_can_process(drv_can_t *drv)
{
    if ((drv == 0) || (drv->rxPending == 0U))
        return;

    drv->rxPending = 0U;
    /* parsing / routing은 Driver가 아니라 Service에서 처리 */
}
