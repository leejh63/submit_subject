#include "uart_hw.h"

#include <stddef.h>

static uint16_t UartHw_RxPending_NextIndex(uint16_t index)
{
    index++;

    if (index >= UART_SERVICE_RX_PENDING_SIZE)
        index = 0U;

    return index;
}

static uint8_t UartHw_RxPending_IsFull(const UartRxPendingBuffer *rxPending)
{
    uint16_t nextTail;

    if (rxPending == NULL)
        return 1U;

    nextTail = UartHw_RxPending_NextIndex(rxPending->tail);

    if (nextTail == rxPending->head)
        return 1U;

    return 0U;
}

static void UartHw_RxPending_Push(UartPort *port, uint8_t rxByte)
{
    UartRxPendingBuffer *rxPending;
    uint16_t nextTail;

    if (port == NULL)
        return;

    rxPending = &port->channel.rxPending;

    if (UartHw_RxPending_IsFull(rxPending) != 0U)
    {
        rxPending->overflow = 1U;
        rxPending->overflowCount++;
        return;
    }

    rxPending->buffer[rxPending->tail] = rxByte;

    nextTail = UartHw_RxPending_NextIndex(rxPending->tail);
    rxPending->tail = nextTail;
}

status_t UartHw_Init(UartPort *port,
                     uint32_t instance,
                     lpuart_state_t *driverState,
                     const lpuart_user_config_t *userConfig)
{
    status_t status;

    if (port == NULL || driverState == NULL || userConfig == NULL)
        return STATUS_ERROR;

    port->instance = instance;
    port->driverState = driverState;
    port->userConfig = userConfig;
    port->rxByte = 0U;

    status = LPUART_DRV_Init(port->instance,
                             port->driverState,
                             port->userConfig);
    if (status != STATUS_SUCCESS)
    {
        port->channel.errorFlag = 1U;
        port->channel.errorCode = UART_ERROR_HW_INIT;
        port->channel.errorCount++;
        return status;
    }

    (void)LPUART_DRV_InstallRxCallback(port->instance,
                                       UartHw_RxCallback,
                                       port);

    status = UartHw_StartReceiveByte(port);
    if (status != STATUS_SUCCESS)
    {
        port->channel.errorFlag = 1U;
        port->channel.errorCode = UART_ERROR_RX_DRIVER;
        port->channel.errorCount++;
        return status;
    }

    return STATUS_SUCCESS;
}

status_t UartHw_StartReceiveByte(UartPort *port)
{
    if (port == NULL)
        return STATUS_ERROR;

    port->rxByte = 0U;

    return LPUART_DRV_ReceiveData(port->instance,
                                  &port->rxByte,
                                  1U);
}

status_t UartHw_ContinueReceiveByte(UartPort *port)
{
    if (port == NULL)
        return STATUS_ERROR;

    port->rxByte = 0U;

    return LPUART_DRV_SetRxBuffer(port->instance,
                                  &port->rxByte,
                                  1U);
}

status_t UartHw_StartTransmit(UartPort *port,
                              const uint8_t *data,
                              uint16_t length)
{
    if (port == NULL || data == NULL || length == 0U)
        return STATUS_ERROR;

    return LPUART_DRV_SendData(port->instance,
                               data,
                               (uint32_t)length);
}

status_t UartHw_GetTransmitStatus(UartPort *port,
                                  uint32_t *bytesRemaining)
{
    if (port == NULL)
        return STATUS_ERROR;

    return LPUART_DRV_GetTransmitStatus(port->instance,
                                        bytesRemaining);
}

void UartHw_RxCallback(void *driverState,
                       uart_event_t event,
                       void *userData)
{
    UartPort *port;
    status_t status;

    (void)driverState;

    if (userData == NULL)
        return;

    port = (UartPort *)userData;

    if (event == UART_EVENT_RX_FULL)
    {
        UartHw_RxPending_Push(port, port->rxByte);

        status = UartHw_ContinueReceiveByte(port);
        if (status != STATUS_SUCCESS)
        {
            port->channel.errorFlag = 1U;
            port->channel.errorCode = UART_ERROR_RX_DRIVER;
            port->channel.errorCount++;
        }
    }
    else if (event == UART_EVENT_ERROR)
    {
        port->channel.errorFlag = 1U;
        port->channel.errorCode = UART_ERROR_RX_DRIVER;
        port->channel.errorCount++;

        status = UartHw_ContinueReceiveByte(port);
        if (status != STATUS_SUCCESS)
            port->channel.errorCount++;
    }
    else
    {
        // do nothing
    }
}
