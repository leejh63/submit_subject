#include "uart_service.h"
#include "uart_hw.h"
#include "runtime_tick.h"

#include <stddef.h>
#include <string.h>

#define UART_CHAR_BACKSPACE_1   ((uint8_t)'\b')
#define UART_CHAR_BACKSPACE_2   (0x7FU)
#define UART_TIME_OUT_VAL       100U

static uint16_t UartService_RxPending_NextIndex(uint16_t index);
static uint8_t UartService_RxPending_IsEmpty(const UartRxPendingBuffer *rxPending);
static status_t UartService_PopPendingRx(UartPort *port, uint8_t *outByte);
static void UartService_ResetRxPending(UartPort *port);
static void UartService_ResetRxLine(UartPort *port);
static void UartService_ResetTx(UartPort *port);
static void UartService_ResetChannel(UartPort *port);
static void UartService_SetError(UartPort *port, UartErrorCode errorCode);
static uint16_t UartService_GetTxFreeSlotCount(const UartPort *port);
static status_t UartService_RequestTxBytes(UartPort *port, const char *msg, uint16_t length);

static uint16_t UartService_RxPending_NextIndex(uint16_t index)
{
    index++;
    if (index >= UART_SERVICE_RX_PENDING_SIZE)
        index = 0U;
    return index;
}

static uint8_t UartService_RxPending_IsEmpty(const UartRxPendingBuffer *rxPending)
{
    if (rxPending == NULL)
        return 1U;

    return (rxPending->head == rxPending->tail) ? 1U : 0U;
}

static status_t UartService_PopPendingRx(UartPort *port, uint8_t *outByte)
{
    UartRxPendingBuffer *rxPending;

    if ((port == NULL) || (outByte == NULL))
        return STATUS_ERROR;

    rxPending = &port->channel.rxPending;
    if (UartService_RxPending_IsEmpty(rxPending) != 0U)
        return STATUS_BUSY;
    *outByte = rxPending->buffer[rxPending->head];
    rxPending->head = UartService_RxPending_NextIndex(rxPending->head);
    return STATUS_SUCCESS;
}

static void UartService_ResetRxPending(UartPort *port)
{
    UartRxPendingBuffer *rxPending;

    if (port == NULL)
        return;

    rxPending = &port->channel.rxPending;
    rxPending->head = rxPending->tail;
    rxPending->overflow = 0U;
}

static void UartService_ResetRxLine(UartPort *port)
{
    if (port == NULL)
        return;

    port->channel.rxLine.length = 0U;
    port->channel.rxLine.lineReady = 0U;
    port->channel.rxLine.overflow = 0U;
    port->channel.rxLine.buffer[0] = '\0';
}

static void UartService_ResetTx(UartPort *port)
{
    status_t status;

    if (port == NULL)
        return;

    port->channel.tx.currentLength = 0U;
    port->channel.tx.busy = 0U;
    port->channel.tx.startTickMs = 0U;
    port->channel.tx.timeoutMs = UART_TIME_OUT_VAL;
    port->channel.tx.currentBuffer[0] = '\0';

    status = MsgQueue_Init(&port->channel.tx.queue,
                           port->channel.tx.queueStorage,
                           UART_SERVICE_TX_QUEUE_SIZE);
    if (status != STATUS_SUCCESS)
        UartService_SetError(port, UART_ERROR_QUEUE_INIT);
}

static void UartService_ResetChannel(UartPort *port)
{
    if (port == NULL)
        return;

    port->channel.errorFlag = 0U;
    port->channel.errorCode = UART_ERROR_NONE;
    port->channel.errorCount = 0U;
    port->channel.rxPending.overflowCount = 0U;

    UartService_ResetRxPending(port);
    UartService_ResetRxLine(port);
    UartService_ResetTx(port);
}

static void UartService_SetError(UartPort *port, UartErrorCode errorCode)
{
    if (port == NULL)
        return;

    port->channel.errorFlag = 1U;
    port->channel.errorCode = errorCode;
    port->channel.errorCount++;
}

static uint16_t UartService_GetTxFreeSlotCount(const UartPort *port)
{
    uint16_t count;
    uint16_t capacity;

    if (port == NULL)
        return 0U;

    count = MsgQueue_GetCount(&port->channel.tx.queue);
    capacity = MsgQueue_GetCapacity(&port->channel.tx.queue);

    if (capacity <= count)
        return 0U;

    return (uint16_t)(capacity - count);
}

static status_t UartService_RequestTxBytes(UartPort *port, const char *msg, uint16_t length)
{
    uint16_t offset;
    uint16_t chunkLength;
    uint16_t requiredSlots;
    uint16_t freeSlots;
    status_t status;

    if ((port == NULL) || (msg == NULL) || (length == 0U))
        return STATUS_ERROR;

    requiredSlots = (uint16_t)((length + UART_SERVICE_TX_CHUNK_SIZE - 1U) / UART_SERVICE_TX_CHUNK_SIZE);
    freeSlots = UartService_GetTxFreeSlotCount(port);

    if (requiredSlots > freeSlots)
        return STATUS_BUSY;

    offset = 0U;
    while (offset < length)
    {
        chunkLength = (uint16_t)(length - offset);
        if (chunkLength > UART_SERVICE_TX_CHUNK_SIZE)
            chunkLength = UART_SERVICE_TX_CHUNK_SIZE;

        status = MsgQueue_Push(&port->channel.tx.queue,
                               (const uint8_t *)&msg[offset],
                               chunkLength);
        if (status != STATUS_SUCCESS)
            return status;

        offset = (uint16_t)(offset + chunkLength);
    }

    return STATUS_SUCCESS;
}

status_t UartService_Init(UartPort *port,
                          uint32_t instance,
                          lpuart_state_t *state,
                          const lpuart_user_config_t *config)
{
    status_t status;

    if ((port == NULL) || (state == NULL) || (config == NULL))
        return STATUS_ERROR;

    UartService_ResetChannel(port);
    if (port->channel.errorFlag != 0U)
        return STATUS_ERROR;

    status = UartHw_Init(port, instance, state, config);
    if (status != STATUS_SUCCESS)
    {
        if (port->channel.errorFlag == 0U)
            UartService_SetError(port, UART_ERROR_HW_INIT);
        return status;
    }

    return STATUS_SUCCESS;
}

void UartService_OnRxByte(UartPort *port, uint8_t rxByte)
{
    UartLineBuffer *rxLine;

    if (port == NULL)
        return;

    rxLine = &port->channel.rxLine;
    if (rxLine->lineReady != 0U)
        return;

    if ((rxByte == '\r') || (rxByte == '\n'))
    {
        if (rxLine->length < UART_SERVICE_RX_BUFFER_SIZE)
            rxLine->buffer[rxLine->length] = '\0';
        else
            rxLine->buffer[UART_SERVICE_RX_BUFFER_SIZE - 1U] = '\0';

        rxLine->lineReady = 1U;
        return;
    }
    // 백스페이스 기능
    if ((rxByte == UART_CHAR_BACKSPACE_1) || (rxByte == UART_CHAR_BACKSPACE_2))
    {
        if (rxLine->length > 0U)
        {
            rxLine->length--;
            rxLine->buffer[rxLine->length] = '\0';
        }
        return;
    }

    // 아스키 문자만 허용
    if ((rxByte < 32U) || (rxByte > 126U))
        return;


    if (rxLine->length >= (UART_SERVICE_RX_BUFFER_SIZE - 1U))
    {
        rxLine->overflow = 1U;
        rxLine->buffer[UART_SERVICE_RX_BUFFER_SIZE - 1U] = '\0';
        rxLine->lineReady = 1U;
        return;
    }

    rxLine->buffer[rxLine->length] = (char)rxByte;
    rxLine->length++;
    rxLine->buffer[rxLine->length] = '\0';
}

uint8_t UartService_HasLine(const UartPort *port)
{
    if (port == NULL)
        return 0U;

    return port->channel.rxLine.lineReady;
}

status_t UartService_GetLine(UartPort *port,
                             char *outBuffer,
                             uint16_t maxLength)
{
    UartLineBuffer *rxLine;
    uint16_t copyLength;

    if ((port == NULL) || (outBuffer == NULL) || (maxLength == 0U))
        return STATUS_ERROR;

    rxLine = &port->channel.rxLine;
    if (rxLine->lineReady == 0U)
        return STATUS_BUSY;

    copyLength = rxLine->length;
    if (copyLength >= maxLength)
        copyLength = (uint16_t)(maxLength - 1U);

    if (copyLength > 0U)
        (void)memcpy(outBuffer, rxLine->buffer, copyLength);

    outBuffer[copyLength] = '\0';
    UartService_ClearRx(port);
    return STATUS_SUCCESS;
}

void UartService_ClearRx(UartPort *port)
{
    UartLineBuffer *rxLine;

    if (port == NULL)
        return;

    rxLine = &port->channel.rxLine;
    rxLine->length = 0U;
    rxLine->lineReady = 0U;
    rxLine->overflow = 0U;
    rxLine->buffer[0] = '\0';
}

status_t UartService_RequestTx(UartPort *port, const char *msg)
{
    uint16_t length;

    if ((port == NULL) || (msg == NULL))
        return STATUS_ERROR;

    length = (uint16_t)strlen(msg);
    if (length == 0U)
        return STATUS_ERROR;

    return UartService_RequestTxBytes(port, msg, length);
}

void UartService_ProcessRx(UartPort *port)
{
    uint8_t rxByte;

    if (port == NULL)
        return;
    // 질문 함수로 만드는게 좋아보이는데 아닌가?
    // rxPending_overflow_check()
    if (port->channel.rxPending.overflow != 0U)
    {
        UartService_ResetRxPending(port);
        UartService_ResetRxLine(port);
        (void)UartService_RequestTx(port, "\r\n[rx pending overflow]\r\n");
        return;
    }

    while (UartService_PopPendingRx(port, &rxByte) == STATUS_SUCCESS)
    {
        UartService_OnRxByte(port, rxByte);

        if (port->channel.rxLine.lineReady != 0U)
            break;

        if (port->channel.rxLine.overflow != 0U)
        {
            UartService_ResetRxLine(port);
            (void)UartService_RequestTx(port, "\r\n[warn] line too long\r\n");
            break;
        }
    }
}

void UartService_ProcessTxWithTick(UartPort *port, uint32_t nowMs)
{
    MsgQueueItem item;
    status_t status;

    if (port == NULL)
        return;

    if (port->channel.tx.busy != 0U)
        return;

    if (MsgQueue_IsEmpty(&port->channel.tx.queue) != 0U)
        return;

    status = MsgQueue_Pop(&port->channel.tx.queue, &item);
    if (status != STATUS_SUCCESS)
        return;

    if (item.length == 0U)
        return;

    if (item.length > UART_SERVICE_TX_CHUNK_SIZE)
        item.length = UART_SERVICE_TX_CHUNK_SIZE;

    (void)memcpy(port->channel.tx.currentBuffer, item.data, item.length);
    port->channel.tx.currentBuffer[item.length] = '\0';
    port->channel.tx.currentLength = item.length;

    status = UartHw_StartTransmit(port,
                                  (const uint8_t *)port->channel.tx.currentBuffer,
                                  port->channel.tx.currentLength);
    if (status != STATUS_SUCCESS)
    {
        port->channel.tx.currentLength = 0U;
        port->channel.tx.currentBuffer[0] = '\0';
        UartService_SetError(port, UART_ERROR_TX_DRIVER);
        return;
    }

    port->channel.tx.startTickMs = nowMs;
    port->channel.tx.busy = 1U;
}

void UartService_UpdateTxWithTick(UartPort *port, uint32_t nowMs)
{
    status_t status;
    uint32_t bytesRemaining;
    uint32_t elapsedMs;

    if (port == NULL)
        return;

    if (port->channel.tx.busy == 0U)
        return;

    elapsedMs = nowMs - port->channel.tx.startTickMs;
    if (elapsedMs >= port->channel.tx.timeoutMs)
    {
        UartService_ClearTx(port);
        UartService_SetError(port, UART_ERROR_TX_TIMEOUT);
        return;
    }

    status = UartHw_GetTransmitStatus(port, &bytesRemaining);
    if ((status == STATUS_SUCCESS) && (bytesRemaining == 0U))
    {
        UartService_ClearTx(port);
    }
    else if (status == STATUS_BUSY)
    {
    }
    else if (status == STATUS_SUCCESS)
    {
    }
    else
    {
        UartService_ClearTx(port);
        UartService_SetError(port, UART_ERROR_TX_DRIVER);
    }
}

void UartService_ProcessTx(UartPort *port)
{
    UartService_ProcessTxWithTick(port, RuntimeTick_GetMs());
}

void UartService_UpdateTx(UartPort *port)
{
    UartService_UpdateTxWithTick(port, RuntimeTick_GetMs());
}

void UartService_ClearTx(UartPort *port)
{
    if (port == NULL)
        return;

    port->channel.tx.currentLength = 0U;
    port->channel.tx.busy = 0U;
    port->channel.tx.startTickMs = 0U;
    port->channel.tx.currentBuffer[0] = '\0';
}

uint8_t UartService_HasError(const UartPort *port)
{
    if (port == NULL)
        return 0U;

    return (port->channel.errorFlag != 0U) ? 1U : 0U;
}

uint8_t UartService_IsTxBusy(const UartPort *port)
{
    if (port == NULL)
        return 0U;

    return (port->channel.tx.busy != 0U) ? 1U : 0U;
}

uint16_t UartService_GetCurrentInputLength(const UartPort *port)
{
    if (port == NULL)
        return 0U;

    return port->channel.rxLine.length;
}

status_t UartService_GetCurrentInputText(const UartPort *port,
                                         char *outBuffer,
                                         uint16_t maxLength)
{
    uint16_t copyLength;

    if ((port == NULL) || (outBuffer == NULL) || (maxLength == 0U))
        return STATUS_ERROR;

    copyLength = port->channel.rxLine.length;
    if (copyLength >= maxLength)
        copyLength = (uint16_t)(maxLength - 1U);

    if (copyLength > 0U)
        (void)memcpy(outBuffer, port->channel.rxLine.buffer, copyLength);

    outBuffer[copyLength] = '\0';
    return STATUS_SUCCESS;
}

uint16_t UartService_GetTxQueueCount(const UartPort *port)
{
    if (port == NULL)
        return 0U;

    return MsgQueue_GetCount(&port->channel.tx.queue);
}

uint16_t UartService_GetTxQueueCapacity(const UartPort *port)
{
    if (port == NULL)
        return 0U;

    return MsgQueue_GetCapacity(&port->channel.tx.queue);
}

UartErrorCode UartService_GetErrorCode(const UartPort *port)
{
    if (port == NULL)
        return UART_ERROR_NONE;

    return port->channel.errorCode;
}

status_t UartService_Recover(UartPort *port)
{
    status_t status;

    if (port == NULL)
        return STATUS_ERROR;

    UartService_ResetChannel(port);
    if (port->channel.errorFlag != 0U)
        return STATUS_ERROR;

    status = UartHw_Init(port,
                         port->instance,
                         port->driverState,
                         port->userConfig);
    if (status != STATUS_SUCCESS)
    {
        if (port->channel.errorFlag == 0U)
            UartService_SetError(port, UART_ERROR_HW_INIT);
        return status;
    }

    return STATUS_SUCCESS;
}
