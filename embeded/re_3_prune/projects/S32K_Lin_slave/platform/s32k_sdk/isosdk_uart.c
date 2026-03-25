#include "isosdk_uart.h"

#include <stddef.h>

#include "lpuart_driver.h"

#include "isosdk_sdk_bindings.h"

#ifdef ISOSDK_SDK_HAS_UART

static IsoSdkUartEventCallback s_iso_sdk_uart_event_cb;
static void                   *s_iso_sdk_uart_event_context;

static void IsoSdk_UartRxCallback(void *driver_state, uart_event_t event, void *user_data)
{
    (void)driver_state;
    (void)user_data;

    if (s_iso_sdk_uart_event_cb == NULL)
    {
        return;
    }

    if (event == UART_EVENT_RX_FULL)
    {
        s_iso_sdk_uart_event_cb(s_iso_sdk_uart_event_context, ISOSDK_UART_EVENT_RX_FULL);
        return;
    }

    if (event == UART_EVENT_ERROR)
    {
        s_iso_sdk_uart_event_cb(s_iso_sdk_uart_event_context, ISOSDK_UART_EVENT_ERROR);
    }
}

uint8_t IsoSdk_UartIsSupported(void)
{
    return 1U;
}

uint32_t IsoSdk_UartGetDefaultInstance(void)
{
    return ISOSDK_SDK_UART_INSTANCE;
}

uint8_t IsoSdk_UartInit(uint32_t instance,
                        IsoSdkUartEventCallback event_cb,
                        void *event_context)
{
    status_t status;

    s_iso_sdk_uart_event_cb = event_cb;
    s_iso_sdk_uart_event_context = event_context;

    status = LPUART_DRV_Init(instance,
                             &ISOSDK_SDK_UART_STATE,
                             &ISOSDK_SDK_UART_INIT_CONFIG);
    if (status != STATUS_SUCCESS)
    {
        return 0U;
    }

    (void)LPUART_DRV_InstallRxCallback(instance, IsoSdk_UartRxCallback, NULL);
    return 1U;
}

uint8_t IsoSdk_UartStartReceiveByte(uint32_t instance, uint8_t *out_byte)
{
    if (out_byte == NULL)
    {
        return 0U;
    }

    *out_byte = 0U;
    return (LPUART_DRV_ReceiveData(instance, out_byte, 1U) == STATUS_SUCCESS) ? 1U : 0U;
}

uint8_t IsoSdk_UartContinueReceiveByte(uint32_t instance, uint8_t *io_byte)
{
    if (io_byte == NULL)
    {
        return 0U;
    }

    *io_byte = 0U;
    return (LPUART_DRV_SetRxBuffer(instance, io_byte, 1U) == STATUS_SUCCESS) ? 1U : 0U;
}

uint8_t IsoSdk_UartStartTransmit(uint32_t instance, const uint8_t *data, uint16_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return 0U;
    }

    return (LPUART_DRV_SendData(instance, data, (uint32_t)length) == STATUS_SUCCESS) ? 1U : 0U;
}

IsoSdkUartTxState IsoSdk_UartGetTransmitState(uint32_t instance, uint32_t *bytes_remaining)
{
    status_t status;
    uint32_t remaining;

    remaining = 0U;
    status = LPUART_DRV_GetTransmitStatus(instance, &remaining);
    if (bytes_remaining != NULL)
    {
        *bytes_remaining = remaining;
    }

    if ((status == STATUS_SUCCESS) && (remaining == 0U))
    {
        return ISOSDK_UART_TX_STATE_DONE;
    }

    if ((status == STATUS_SUCCESS) || (status == STATUS_BUSY))
    {
        return ISOSDK_UART_TX_STATE_BUSY;
    }

    return ISOSDK_UART_TX_STATE_ERROR;
}

#else

uint8_t IsoSdk_UartIsSupported(void)
{
    return 0U;
}

uint32_t IsoSdk_UartGetDefaultInstance(void)
{
    return 0U;
}

uint8_t IsoSdk_UartInit(uint32_t instance,
                        IsoSdkUartEventCallback event_cb,
                        void *event_context)
{
    (void)instance;
    (void)event_cb;
    (void)event_context;
    return 0U;
}

uint8_t IsoSdk_UartStartReceiveByte(uint32_t instance, uint8_t *out_byte)
{
    (void)instance;
    (void)out_byte;
    return 0U;
}

uint8_t IsoSdk_UartContinueReceiveByte(uint32_t instance, uint8_t *io_byte)
{
    (void)instance;
    (void)io_byte;
    return 0U;
}

uint8_t IsoSdk_UartStartTransmit(uint32_t instance, const uint8_t *data, uint16_t length)
{
    (void)instance;
    (void)data;
    (void)length;
    return 0U;
}

IsoSdkUartTxState IsoSdk_UartGetTransmitState(uint32_t instance, uint32_t *bytes_remaining)
{
    (void)instance;

    if (bytes_remaining != NULL)
    {
        *bytes_remaining = 0U;
    }

    return ISOSDK_UART_TX_STATE_ERROR;
}

#endif
