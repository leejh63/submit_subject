/*
 * generated SDK driver 위에 만든 UART 하드웨어 바인딩 구현부다.
 * 바이트 단위 RX callback 처리는 여기서 맡고,
 * line 조립과 정책 판단은 UartService에 넘긴다.
 */
#include "uart_hw.h"

#include <stddef.h>

#include "sdk_project_config.h"

#ifdef INST_LPUART_1

/*
 * RX pending ring 안에서 index를 한 칸 전진시킨다.
 * 이 로직을 지역화해 두면,
 * callback 경로가 작아지고 wrap-around 계산 중복도 사라진다.
 */
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

static status_t UartHw_StartReceiveByte(UartService *service)
{
    if (service == NULL)
    {
        return STATUS_ERROR;
    }

    service->rx_byte = 0U;
    return LPUART_DRV_ReceiveData(service->instance, &service->rx_byte, 1U);
}

static status_t UartHw_ContinueReceiveByte(UartService *service)
{
    if (service == NULL)
    {
        return STATUS_ERROR;
    }

    service->rx_byte = 0U;
    return LPUART_DRV_SetRxBuffer(service->instance, &service->rx_byte, 1U);
}

/*
 * 기본 콘솔 용도로 generated UART driver를 초기화한다.
 * 하드웨어 계층이 generated instance를 선택하고,
 * UartService가 사용할 바이트 단위 RX callback을 설치한다.
 */
status_t UartHw_InitDefault(UartService *service)
{
    status_t status;

    if (service == NULL)
    {
        return STATUS_ERROR;
    }

    /*
     * 사용자 바인딩 확인:
     * 생성 코드에서 UART 주변장치 이름이 바뀌면,
     * 이 파일이 참조하는 바인딩 심볼도 함께 수정해야 한다.
     */
    service->instance = INST_LPUART_1;
    status = LPUART_DRV_Init(service->instance, &lpUartState1, &lpuart_1_InitConfig0);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    (void)LPUART_DRV_InstallRxCallback(service->instance, UartHw_RxCallback, service);
    return UartHw_StartReceiveByte(service);
}

/*
 * generated UART driver를 통해 TX 전송 하나를 시작한다.
 * service 계층은 소프트웨어 관리 transmit buffer에서,
 * 다음 chunk를 고른 뒤 이 함수를 사용한다.
 */
status_t UartHw_StartTransmit(UartService *service, const uint8_t *data, uint16_t length)
{
    if ((service == NULL) || (data == NULL) || (length == 0U))
    {
        return STATUS_ERROR;
    }

    return LPUART_DRV_SendData(service->instance, data, (uint32_t)length);
}

status_t UartHw_GetTransmitStatus(UartService *service, uint32_t *bytes_remaining)
{
    if (service == NULL)
    {
        return STATUS_ERROR;
    }

    return LPUART_DRV_GetTransmitStatus(service->instance, bytes_remaining);
}

/*
 * 수신 바이트와 오류에 대한 UART driver callback을 처리한다.
 * callback은 바이트 버퍼링과 오류 flag 갱신만 수행해,
 * 나중에 service 계층이 이어서 처리하도록 일을 최소화한다.
 */
void UartHw_RxCallback(void *driver_state, uart_event_t event, void *user_data)
{
    UartService *service;
    status_t     status;

    (void)driver_state;

    if (user_data == NULL)
    {
        return;
    }

    service = (UartService *)user_data;

    if (event == UART_EVENT_RX_FULL)
    {
        UartHw_PushPendingByte(service, service->rx_byte);
        status = UartHw_ContinueReceiveByte(service);
        if (status != STATUS_SUCCESS)
        {
            service->error_flag = 1U;
            service->error_code = UART_ERROR_RX_DRIVER;
            service->error_count++;
        }
        return;
    }

    if (event == UART_EVENT_ERROR)
    {
        service->error_flag = 1U;
        service->error_code = UART_ERROR_RX_DRIVER;
        service->error_count++;

        status = UartHw_ContinueReceiveByte(service);
        if (status != STATUS_SUCCESS)
        {
            service->error_count++;
        }
    }
}

#else

status_t UartHw_InitDefault(UartService *service)
{
    (void)service;
    return STATUS_ERROR;
}

status_t UartHw_StartTransmit(UartService *service, const uint8_t *data, uint16_t length)
{
    (void)service;
    (void)data;
    (void)length;
    return STATUS_ERROR;
}

status_t UartHw_GetTransmitStatus(UartService *service, uint32_t *bytes_remaining)
{
    (void)service;

    if (bytes_remaining != NULL)
    {
        *bytes_remaining = 0U;
    }

    return STATUS_ERROR;
}

void UartHw_RxCallback(void *driver_state, uart_event_t event, void *user_data)
{
    (void)driver_state;
    (void)event;
    (void)user_data;
}

#endif
