#include <string.h>
#include "drivers/drv_uart.h"

static uint8_t DrvUart_NextIndex(uint8_t index, uint8_t size)
{
    index++;
    if (index >= size)
        index = 0U;
    return index;
}

static EmbResult DrvUart_PushRxByte(DrvUart *uart, uint8_t byte)
{
    if (uart == 0)
        return EMB_EINVAL;

    if (uart->rxCount >= DRV_UART_RX_RING_SIZE)
    {
        uart->rxOverflowCount++;
        uart->lastError = EMB_ENOSPACE;
        return EMB_ENOSPACE;
    }

    uart->rxRing[uart->rxTail] = byte;
    uart->rxTail = DrvUart_NextIndex(uart->rxTail, DRV_UART_RX_RING_SIZE);
    uart->rxCount++;
    uart->rxByteCount++;
    return EMB_OK;
}

EmbResult DrvUart_Init(DrvUart *uart, const DrvUartConfig *config)
{
    if (uart == 0 || config == 0)
        return EMB_EINVAL;

    (void)memset(uart, 0, sizeof(*uart));
    uart->config = *config;
    uart->lastError = EMB_OK;
    return EMB_OK;
}

EmbResult DrvUart_Start(DrvUart *uart)
{
    if (uart == 0)
        return EMB_EINVAL;

    if (HalS32kUart_Init(&uart->config.port) != EMB_OK)
    {
        uart->lastError = EMB_EIO;
        return EMB_EIO;
    }

    /* RX arm 은 HAL PollRxByte 내부에서 내부 staging byte 기준으로 시작한다. */
    uart->started = 1U;
    return EMB_OK;
}

EmbResult DrvUart_Process(DrvUart *uart)
{
    uint8_t byte;
    uint8_t txBusy;
    EmbResult status;

    if (uart == 0 || uart->started == 0U)
        return EMB_ESTATE;

    for (;;)
    {
        status = HalS32kUart_PollRxByte(&uart->config.port, &byte);
        if (status == EMB_OK)
        {
            (void)DrvUart_PushRxByte(uart, byte);
            continue;
        }
        if (status != EMB_EBUSY && status != EMB_EUNSUPPORTED)
        {
            uart->errorCount++;
            uart->lastError = status;
        }
        break;
    }

    if (uart->txInFlight != 0U)
    {
        status = HalS32kUart_IsTxBusy(&uart->config.port, &txBusy);
        if (status == EMB_OK)
        {
            if (txBusy == 0U)
            {
                uart->txInFlight = 0U;
                uart->txCompleteCount++;
            }
        }
        else if (status != EMB_EUNSUPPORTED)
        {
            uart->errorCount++;
            uart->lastError = status;
            uart->txInFlight = 0U;
        }
    }

    if ((uart->txInFlight == 0U) && (uart->txCount > 0U))
    {
        status = HalS32kUart_Tx(&uart->config.port,
                                uart->txQueue[uart->txHead],
                                uart->txSize[uart->txHead]);
        if (status == EMB_OK)
        {
            uart->txHead = DrvUart_NextIndex(uart->txHead, DRV_UART_TX_QUEUE_DEPTH);
            uart->txCount--;
            uart->txInFlight = 1U;
            uart->txStartCount++;
        }
        else if (status != EMB_EUNSUPPORTED)
        {
            uart->errorCount++;
            uart->lastError = status;
        }
    }

    return EMB_OK;
}

EmbResult DrvUart_Send(DrvUart *uart, const uint8_t *data, uint32_t size)
{
    uint32_t i;

    if (uart == 0 || data == 0 || size == 0U)
        return EMB_EINVAL;
    if (uart->started == 0U)
        return EMB_ESTATE;
    if (size > DRV_UART_TX_CHUNK_SIZE)
        return EMB_EINVAL;
    if (uart->txCount >= DRV_UART_TX_QUEUE_DEPTH)
    {
        uart->txDropCount++;
        uart->lastError = EMB_ENOSPACE;
        return EMB_ENOSPACE;
    }

    for (i = 0U; i < size; i++)
        uart->txQueue[uart->txTail][i] = data[i];

    uart->txSize[uart->txTail] = (uint8_t)size;
    uart->txTail = DrvUart_NextIndex(uart->txTail, DRV_UART_TX_QUEUE_DEPTH);
    uart->txCount++;
    return EMB_OK;
}

EmbResult DrvUart_TryReadByte(DrvUart *uart, uint8_t *outByte)
{
    if (uart == 0 || outByte == 0)
        return EMB_EINVAL;
    if (uart->started == 0U)
        return EMB_ESTATE;
    if (uart->rxCount == 0U)
        return EMB_EBUSY;

    *outByte = uart->rxRing[uart->rxHead];
    uart->rxHead = DrvUart_NextIndex(uart->rxHead, DRV_UART_RX_RING_SIZE);
    uart->rxCount--;
    return EMB_OK;
}
