/*
 * IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다.
 * 바이트 단위 RX callback 처리는 여기서 맡고,
 * line 조립과 정책 판단은 UartService에 넘긴다.
 */
#include "uart_hw.h"

#include <stddef.h>

#include "../../platform/s32k_sdk/isosdk_uart.h"

static UartHwStatus UartHw_StatusFromBool(uint8_t ok)
{
    return (ok != 0U) ? UART_HW_STATUS_OK : UART_HW_STATUS_ERROR;
}

static uint16_t UartHw_NextPendingIndex(uint16_t index)
{
    index++;
    if (index >= UART_RX_PENDING_SIZE)
    {
        index = 0U;
    }

    return index;
}

static uint8_t UartHw_IsPendingFull(const UartRxPendingRing *ring)
{
    uint16_t next_tail;

    if (ring == NULL)
    {
        return 1U;
    }

    next_tail = UartHw_NextPendingIndex(ring->tail);
    return (next_tail == ring->head) ? 1U : 0U;
}

static void UartHw_PushPendingByte(UartService *service, uint8_t rx_byte)
{
    UartRxPendingRing *ring;

    if (service == NULL)
    {
        return;
    }

    ring = &service->rx_pending;
    if (UartHw_IsPendingFull(ring) != 0U)
    {
        ring->overflow = 1U;
        ring->overflow_count++;
        return;
    }

    ring->buffer[ring->tail] = rx_byte;
    ring->tail = UartHw_NextPendingIndex(ring->tail);
}

static UartHwStatus UartHw_StartReceiveByte(UartService *service)
{
    if (service == NULL)
    {
        return UART_HW_STATUS_ERROR;
    }

    return UartHw_StatusFromBool(IsoSdk_UartStartReceiveByte(service->instance,
                                                             &service->rx_byte));
}

static UartHwStatus UartHw_ContinueReceiveByte(UartService *service)
{
    if (service == NULL)
    {
        return UART_HW_STATUS_ERROR;
    }

    return UartHw_StatusFromBool(IsoSdk_UartContinueReceiveByte(service->instance,
                                                                &service->rx_byte));
}

static void UartHw_OnIsoSdkEvent(void *context, uint8_t event_id)
{
    UartService  *service;
    UartHwStatus  hw_status;

    service = (UartService *)context;
    if (service == NULL)
    {
        return;
    }

    if (event_id == ISOSDK_UART_EVENT_RX_FULL)
    {
        UartHw_PushPendingByte(service, service->rx_byte);
        hw_status = UartHw_ContinueReceiveByte(service);
        if (hw_status != UART_HW_STATUS_OK)
        {
            service->error_flag = 1U;
            service->error_code = UART_ERROR_RX_DRIVER;
            service->error_count++;
        }
        return;
    }

    if (event_id == ISOSDK_UART_EVENT_ERROR)
    {
        service->error_flag = 1U;
        service->error_code = UART_ERROR_RX_DRIVER;
        service->error_count++;

        hw_status = UartHw_ContinueReceiveByte(service);
        if (hw_status != UART_HW_STATUS_OK)
        {
            service->error_count++;
        }
    }
}

UartHwStatus UartHw_InitDefault(UartService *service)
{
    if (service == NULL)
    {
        return UART_HW_STATUS_ERROR;
    }

    if (IsoSdk_UartIsSupported() == 0U)
    {
        return UART_HW_STATUS_ERROR;
    }

    service->instance = IsoSdk_UartGetDefaultInstance();
    if (IsoSdk_UartInit(service->instance, UartHw_OnIsoSdkEvent, service) == 0U)
    {
        return UART_HW_STATUS_ERROR;
    }

    return UartHw_StartReceiveByte(service);
}

UartHwStatus UartHw_StartTransmit(UartService *service, const uint8_t *data, uint16_t length)
{
    if ((service == NULL) || (data == NULL) || (length == 0U))
    {
        return UART_HW_STATUS_ERROR;
    }

    return UartHw_StatusFromBool(IsoSdk_UartStartTransmit(service->instance, data, length));
}

UartHwStatus UartHw_GetTransmitStatus(UartService *service, uint32_t *bytes_remaining)
{
    IsoSdkUartTxState tx_state;

    if (service == NULL)
    {
        return UART_HW_STATUS_ERROR;
    }

    tx_state = IsoSdk_UartGetTransmitState(service->instance, bytes_remaining);
    if (tx_state == ISOSDK_UART_TX_STATE_DONE)
    {
        return UART_HW_STATUS_OK;
    }

    if (tx_state == ISOSDK_UART_TX_STATE_BUSY)
    {
        return UART_HW_STATUS_BUSY;
    }

    return UART_HW_STATUS_ERROR;
}
