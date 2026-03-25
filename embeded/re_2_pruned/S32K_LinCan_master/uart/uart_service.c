/*
 * 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다.
 * 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며,
 * 전송 오류를 모듈 친화적인 형태로 추적한다.
 */
#include "uart_service.h"

#include <stddef.h>
#include <string.h>

#include "uart_hw.h"

#define UART_CHAR_BACKSPACE_1  ((uint8_t)'\b')
#define UART_CHAR_BACKSPACE_2  (0x7FU)

/*
 * service 수준 UART 오류 하나를 기록한다.
 * 오류 bookkeeping을 한곳에 모아 두면,
 * 모든 실패 경로가 같은 flag, code, counter 의미를 유지한다.
 */
static void UartService_SetError(UartService *service, UartErrorCode code)
{
    if (service == NULL)
    {
        return;
    }

    service->error_flag = 1U;
    service->error_code = code;
    service->error_count++;
}

/*
 * 버퍼링된 수신 상태를 모두 초기화한다.
 * startup과 recovery, overflow 처리 때 사용하여,
 * 다음 line이 깨끗한 parser 상태에서 시작되게 만든다.
 */
static void UartService_ResetRx(UartService *service)
{
    if (service == NULL)
    {
        return;
    }

    service->rx_pending.head = 0U;
    service->rx_pending.tail = 0U;
    service->rx_pending.overflow = 0U;
    service->rx_pending.overflow_count = 0U;

    service->rx_line.length = 0U;
    service->rx_line.line_ready = 0U;
    service->rx_line.overflow = 0U;
    service->rx_line.buffer[0] = '\0';
}

static InfraStatus UartService_ResetTx(UartService *service)
{
    if (service == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    service->tx.current_length = 0U;
    service->tx.current_buffer[0] = '\0';
    service->tx.busy = 0U;
    service->tx.start_ms = 0U;
    service->tx.timeout_ms = UART_DEFAULT_TIMEOUT_MS;

    return InfraQueue_Init(&service->tx.queue,
                           service->tx.queue_storage,
                           (uint16_t)sizeof(UartTxChunk),
                           UART_TX_QUEUE_SIZE);
}

static void UartService_Reset(UartService *service)
{
    if (service == NULL)
    {
        return;
    }

    (void)memset(service, 0, sizeof(*service));
    UartService_ResetRx(service);
    if (UartService_ResetTx(service) != INFRA_STATUS_OK)
    {
        UartService_SetError(service, UART_ERROR_TX_QUEUE_FULL);
    }
}

static uint16_t UartService_NextPendingIndex(uint16_t index)
{
    index++;
    if (index >= UART_RX_PENDING_SIZE)
    {
        index = 0U;
    }

    return index;
}

static uint8_t UartService_IsPendingEmpty(const UartService *service)
{
    if (service == NULL)
    {
        return 1U;
    }

    return (service->rx_pending.head == service->rx_pending.tail) ? 1U : 0U;
}

static InfraStatus UartService_PopPendingByte(UartService *service, uint8_t *out_byte)
{
    if ((service == NULL) || (out_byte == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (UartService_IsPendingEmpty(service) != 0U)
    {
        return INFRA_STATUS_EMPTY;
    }

    *out_byte = service->rx_pending.buffer[service->rx_pending.head];
    service->rx_pending.head = UartService_NextPendingIndex(service->rx_pending.head);
    return INFRA_STATUS_OK;
}

/*
 * 수신 바이트 하나를 현재 line buffer에 넣는다.
 * parser는 newline 종료와 backspace 편집,
 * printable 필터링과 overflow 감지를 이 한곳에서 처리한다.
 */
static void UartService_OnRxByte(UartService *service, uint8_t rx_byte)
{
    UartLineBuffer *line;

    if (service == NULL)
    {
        return;
    }

    line = &service->rx_line;
    if (line->line_ready != 0U)
    {
        return;
    }

    if ((rx_byte == '\r') || (rx_byte == '\n'))
    {
        if (line->length < UART_RX_LINE_SIZE)
        {
            line->buffer[line->length] = '\0';
        }
        else
        {
            line->buffer[UART_RX_LINE_SIZE - 1U] = '\0';
        }

        line->line_ready = 1U;
        return;
    }

    if ((rx_byte == UART_CHAR_BACKSPACE_1) || (rx_byte == UART_CHAR_BACKSPACE_2))
    {
        if (line->length > 0U)
        {
            line->length--;
            line->buffer[line->length] = '\0';
        }
        return;
    }

    if ((rx_byte < 32U) || (rx_byte > 126U))
    {
        return;
    }

    if (line->length >= (UART_RX_LINE_SIZE - 1U))
    {
        line->overflow = 1U;
        line->buffer[UART_RX_LINE_SIZE - 1U] = '\0';
        line->line_ready = 1U;
        return;
    }

    line->buffer[line->length] = (char)rx_byte;
    line->length++;
    line->buffer[line->length] = '\0';
}

/*
 * 임의의 바이트를 나중에 보낼 UART 전송 queue에 올린다.
 * 긴 텍스트 블록은 제한된 크기의 chunk로 나뉘어,
 * driver가 service 계층에서 관리하기 쉬운 전송만 보게 된다.
 */
static InfraStatus UartService_RequestTxBytes(UartService *service,
                                              const uint8_t *data,
                                              uint16_t length)
{
    UartTxChunk  chunk;
    uint16_t     offset;
    uint16_t     chunk_length;
    InfraStatus  status;

    if ((service == NULL) || (data == NULL) || (length == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    offset = 0U;
    while (offset < length)
    {
        chunk_length = (uint16_t)(length - offset);
        if (chunk_length > UART_TX_CHUNK_SIZE)
        {
            chunk_length = UART_TX_CHUNK_SIZE;
        }

        (void)memset(&chunk, 0, sizeof(chunk));
        (void)memcpy(chunk.data, &data[offset], chunk_length);
        chunk.length = chunk_length;

        status = InfraQueue_Push(&service->tx.queue, &chunk);
        if (status != INFRA_STATUS_OK)
        {
            UartService_SetError(service, UART_ERROR_TX_QUEUE_FULL);
            return status;
        }

        offset = (uint16_t)(offset + chunk_length);
    }

    return INFRA_STATUS_OK;
}

/*
 * UART service와 저수준 하드웨어 바인딩을 초기화한다.
 * 먼저 소프트웨어 버퍼를 reset해 두어,
 * 하드웨어 실패가 나도 service가 정의된 진단 상태를 유지하게 한다.
 */
InfraStatus UartService_Init(UartService *service)
{
    UartHwStatus status;

    if (service == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    UartService_Reset(service);
    if (service->error_flag != 0U)
    {
        return INFRA_STATUS_IO_ERROR;
    }

    status = UartHw_InitDefault(service);
    if (status != UART_HW_STATUS_OK)
    {
        UartService_SetError(service, UART_ERROR_HW_INIT);
        return INFRA_STATUS_IO_ERROR;
    }

    service->initialized = 1U;
    return INFRA_STATUS_OK;
}

/*
 * 치명적인 UART 오류 이후 service를 다시 초기화한다.
 * console은 전체 펌웨어 reset 대신,
 * 수동 `recover` 명령에서 이 경로를 사용한다.
 */
InfraStatus UartService_Recover(UartService *service)
{
    return UartService_Init(service);
}

/*
 * pending RX 바이트를 비우며 다음 완성 line을 만든다.
 * ISR 작업량은 작게 유지하면서도,
 * line parsing은 더 명확한 일반 scheduler context에서 수행되게 한다.
 */
void UartService_ProcessRx(UartService *service)
{
    uint8_t rx_byte;

    if ((service == NULL) || (service->initialized == 0U))
    {
        return;
    }

    if (service->rx_pending.overflow != 0U)
    {
        UartService_ResetRx(service);
        UartService_SetError(service, UART_ERROR_RX_PENDING_OVERFLOW);
        return;
    }

    while (UartService_PopPendingByte(service, &rx_byte) == INFRA_STATUS_OK)
    {
        UartService_OnRxByte(service, rx_byte);

        if (service->rx_line.overflow != 0U)
        {
            UartService_ResetRx(service);
            UartService_SetError(service, UART_ERROR_RX_LINE_OVERFLOW);
            break;
        }

        if (service->rx_line.line_ready != 0U)
        {
            break;
        }
    }
}

/*
 * 현재 transmit 작업을 진행시키거나 다음 작업을 시작한다.
 * timeout 감지도 여기서 수행하여,
 * scheduler를 막지 않고 stalled driver transfer를 표시할 수 있게 한다.
 */
void UartService_ProcessTx(UartService *service, uint32_t now_ms)
{
    InfraStatus status;
    UartTxChunk chunk;
    uint32_t    bytes_remaining;
    UartHwStatus    hw_status;

    if ((service == NULL) || (service->initialized == 0U))
    {
        return;
    }

    if (service->tx.busy == 0U)
    {
        status = InfraQueue_Pop(&service->tx.queue, &chunk);
        if (status == INFRA_STATUS_EMPTY)
        {
            return;
        }

        if (status != INFRA_STATUS_OK)
        {
            UartService_SetError(service, UART_ERROR_TX_QUEUE_FULL);
            return;
        }

        if (chunk.length == 0U)
        {
            return;
        }

        (void)memcpy(service->tx.current_buffer, chunk.data, chunk.length);
        service->tx.current_buffer[chunk.length] = '\0';
        service->tx.current_length = chunk.length;

        hw_status = UartHw_StartTransmit(service,
                                         (const uint8_t *)service->tx.current_buffer,
                                         service->tx.current_length);
        if (hw_status != UART_HW_STATUS_OK)
        {
            service->tx.current_length = 0U;
            service->tx.current_buffer[0] = '\0';
            UartService_SetError(service, UART_ERROR_TX_DRIVER);
            return;
        }

        service->tx.busy = 1U;
        service->tx.start_ms = now_ms;
        return;
    }

    if (Infra_TimeIsExpired(now_ms, service->tx.start_ms, service->tx.timeout_ms) != 0U)
    {
        service->tx.busy = 0U;
        service->tx.current_length = 0U;
        service->tx.current_buffer[0] = '\0';
        UartService_SetError(service, UART_ERROR_TX_TIMEOUT);
        return;
    }

    bytes_remaining = 0U;
    hw_status = UartHw_GetTransmitStatus(service, &bytes_remaining);
    if ((hw_status == UART_HW_STATUS_OK) && (bytes_remaining == 0U))
    {
        service->tx.busy = 0U;
        service->tx.current_length = 0U;
        service->tx.current_buffer[0] = '\0';
        return;
    }

    if (hw_status == UART_HW_STATUS_BUSY)
    {
        return;
    }

    if (hw_status != UART_HW_STATUS_OK)
    {
        service->tx.busy = 0U;
        service->tx.current_length = 0U;
        service->tx.current_buffer[0] = '\0';
        UartService_SetError(service, UART_ERROR_TX_DRIVER);
    }
}

/*
 * NUL 종료 문자열 하나를 전송 queue에 올린다.
 * 상위 console 계층은 모든 terminal 갱신에서,
 * 일반 렌더링과 메시지 출력 경로로 이 함수를 사용한다.
 */
InfraStatus UartService_RequestTx(UartService *service, const char *text)
{
    uint16_t length;

    if ((service == NULL) || (text == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    length = (uint16_t)strlen(text);
    if (length == 0U)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    return UartService_RequestTxBytes(service, (const uint8_t *)text, length);
}

uint8_t UartService_HasLine(const UartService *service)
{
    if (service == NULL)
    {
        return 0U;
    }

    return service->rx_line.line_ready;
}

/*
 * 현재 완성된 line을 caller 버퍼로 복사한다.
 * line을 읽어가면 RX 조립 상태도 함께 reset되어,
 * 다음 operator 명령이 빈 buffer에서 시작된다.
 */
InfraStatus UartService_ReadLine(UartService *service, char *out_buffer, uint16_t max_length)
{
    uint16_t copy_length;

    if ((service == NULL) || (out_buffer == NULL) || (max_length == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (service->rx_line.line_ready == 0U)
    {
        return INFRA_STATUS_EMPTY;
    }

    copy_length = service->rx_line.length;
    if (copy_length >= max_length)
    {
        copy_length = (uint16_t)(max_length - 1U);
    }

    if (copy_length > 0U)
    {
        (void)memcpy(out_buffer, service->rx_line.buffer, copy_length);
    }

    out_buffer[copy_length] = '\0';
    UartService_ResetRx(service);
    return INFRA_STATUS_OK;
}

InfraStatus UartService_GetCurrentInputText(const UartService *service,
                                            char *out_buffer,
                                            uint16_t max_length)
{
    uint16_t copy_length;

    if ((service == NULL) || (out_buffer == NULL) || (max_length == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    copy_length = service->rx_line.length;
    if (copy_length >= max_length)
    {
        copy_length = (uint16_t)(max_length - 1U);
    }

    if (copy_length > 0U)
    {
        (void)memcpy(out_buffer, service->rx_line.buffer, copy_length);
    }

    out_buffer[copy_length] = '\0';
    return INFRA_STATUS_OK;
}

uint16_t UartService_GetCurrentInputLength(const UartService *service)
{
    if (service == NULL)
    {
        return 0U;
    }

    return service->rx_line.length;
}

uint16_t UartService_GetTxQueueCount(const UartService *service)
{
    if (service == NULL)
    {
        return 0U;
    }

    return InfraQueue_GetCount(&service->tx.queue);
}

uint16_t UartService_GetTxQueueCapacity(const UartService *service)
{
    if (service == NULL)
    {
        return 0U;
    }

    return InfraQueue_GetCapacity(&service->tx.queue);
}

uint8_t UartService_IsTxBusy(const UartService *service)
{
    if (service == NULL)
    {
        return 0U;
    }

    return service->tx.busy;
}

uint8_t UartService_HasError(const UartService *service)
{
    if (service == NULL)
    {
        return 1U;
    }

    return service->error_flag;
}

UartErrorCode UartService_GetErrorCode(const UartService *service)
{
    if (service == NULL)
    {
        return UART_ERROR_HW_INIT;
    }

    return service->error_code;
}
