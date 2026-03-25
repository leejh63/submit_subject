#include "drivers/drv_uart.h"

emb_status_t drv_uart_init(drv_uart_t *drv, const drv_uart_config_t *config)
{
    emb_status_t status;

    if ((drv == 0) || (config == 0) || (config->hal == 0) || (config->hal->ops == 0))
        return EMB_EINVAL;

    drv->config = *config;
    drv->state = EMB_MODULE_STATE_READY;
    drv->stats = (emb_fault_stats_t){0};
    drv->rxPending = 0U;
    drv->txBusy = 0U;

    status = emb_ring_u8_init(&drv->rxRing, config->rxStorage, config->rxCapacity);
    if (status != EMB_OK)
        return status;

    return drv->config.hal->ops->configure(drv->config.hal, drv->config.baudrate);
}

emb_status_t drv_uart_start(drv_uart_t *drv)
{
    if (drv == 0)
        return EMB_EINVAL;

    drv->state = EMB_MODULE_STATE_RUNNING;
    return drv->config.hal->ops->start_rx_irq(drv->config.hal);
}

emb_status_t drv_uart_send_irq(drv_uart_t *drv, const uint8_t *data, size_t length)
{
    if ((drv == 0) || (data == 0) || (length == 0U))
        return EMB_EINVAL;
    if (drv->txBusy != 0U)
        return EMB_EBUSY;

    drv->txBusy = 1U;
    return drv->config.hal->ops->start_tx_irq(drv->config.hal, data, length);
}

emb_status_t drv_uart_send_dma(drv_uart_t *drv, const uint8_t *data, size_t length)
{
    if ((drv == 0) || (data == 0) || (length == 0U))
        return EMB_EINVAL;
    if (drv->txBusy != 0U)
        return EMB_EBUSY;

    drv->txBusy = 1U;
    return drv->config.hal->ops->start_tx_dma(drv->config.hal, data, length);
}

emb_status_t drv_uart_read_byte(drv_uart_t *drv, uint8_t *outByte)
{
    if ((drv == 0) || (outByte == 0))
        return EMB_EINVAL;

    return emb_ring_u8_pop(&drv->rxRing, outByte);
}

void drv_uart_on_rx_byte_irq(drv_uart_t *drv, uint8_t byte)
{
    if (drv == 0)
        return;

    if (emb_ring_u8_push(&drv->rxRing, byte) != EMB_OK)
        drv->stats.dropCount++;

    drv->rxPending = 1U;
}

void drv_uart_on_tx_done_irq(drv_uart_t *drv)
{
    if (drv == 0)
        return;

    drv->txBusy = 0U;
}

void drv_uart_process(drv_uart_t *drv)
{
    if ((drv == 0) || (drv->rxPending == 0U))
        return;

    drv->rxPending = 0U;

    if (drv->config.rxReadyCb != 0)
        drv->config.rxReadyCb(drv->config.context);
}
