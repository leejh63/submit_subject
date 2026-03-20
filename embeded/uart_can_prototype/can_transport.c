#include "can_transport.h"

#include <stddef.h>
#include <string.h>

static void CanTransport_ClearFrame(CanFrame *frame)
{
    if (frame == NULL)
        return;

    (void)memset(frame, 0, sizeof(*frame));
}

static uint8_t CanTransport_TxNext(uint8_t index)
{
    index++;
    if (index >= CAN_TRANSPORT_TX_QUEUE_SIZE)
        index = 0U;
    return index;
}

static uint8_t CanTransport_RxNext(uint8_t index)
{
    index++;
    if (index >= CAN_TRANSPORT_RX_QUEUE_SIZE)
        index = 0U;
    return index;
}

static uint8_t CanTransport_TxIsFull(const CanTransport *transport)
{
    if (transport == NULL)
        return 1U;

    return (transport->txCount >= CAN_TRANSPORT_TX_QUEUE_SIZE) ? 1U : 0U;
}

static uint8_t CanTransport_RxIsFull(const CanTransport *transport)
{
    if (transport == NULL)
        return 1U;

    return (transport->rxCount >= CAN_TRANSPORT_RX_QUEUE_SIZE) ? 1U : 0U;
}

static uint8_t CanTransport_TxPeek(const CanTransport *transport, CanFrame *outFrame)
{
    if (transport == NULL || outFrame == NULL)
        return 0U;

    if (transport->txCount == 0U)
        return 0U;

    *outFrame = transport->txQueue[transport->txHead];
    return 1U;
}

static void CanTransport_TxDropFront(CanTransport *transport);

static void CanTransport_OnTxComplete(CanTransport *transport)
{
    if (transport == NULL)
        return;

    if (transport->hw.txOkCount != transport->hwTxOkCountSeen)
    {
        transport->hwTxOkCountSeen = transport->hw.txOkCount;
        transport->txInFlight = 0U;
        CanTransport_TxDropFront(transport);
        CanTransport_ClearFrame(&transport->currentTxFrame);
        return;
    }

    if (transport->hw.txErrorCount != transport->hwTxErrorCountSeen)
    {
        transport->hwTxErrorCountSeen = transport->hw.txErrorCount;
        transport->txInFlight = 0U;
        transport->txRetryCount++;
        transport->lastError = CAN_TRANSPORT_ERROR_HW_TX_FAIL;
        CanTransport_ClearFrame(&transport->currentTxFrame);
        return;
    }

    transport->txInFlight = 0U;
    transport->lastError = CAN_TRANSPORT_ERROR_HW_TX_FAIL;
    CanTransport_ClearFrame(&transport->currentTxFrame);
}

static void CanTransport_TxDropFront(CanTransport *transport)
{
    if (transport == NULL || transport->txCount == 0U)
        return;

    CanTransport_ClearFrame(&transport->txQueue[transport->txHead]);
    transport->txHead = CanTransport_TxNext(transport->txHead);
    transport->txCount--;
}

static uint8_t CanTransport_RxPush(CanTransport *transport, const CanFrame *frame)
{
    if (transport == NULL || frame == NULL)
        return 0U;

    if (CanTransport_RxIsFull(transport) != 0U)
        return 0U;

    transport->rxQueue[transport->rxTail] = *frame;
    transport->rxTail = CanTransport_RxNext(transport->rxTail);
    transport->rxCount++;
    return 1U;
}

static void CanTransport_DrainHwRx(CanTransport *transport)
{
    CanFrame frame;

    if (transport == NULL)
        return;

    while (CanHw_TryPopRx(&transport->hw, &frame) != 0U)
    {
        if (CanTransport_RxPush(transport, &frame) != 0U)
        {
            transport->rxQueuedCount++;
        }
        else
        {
            transport->rxDropCount++;
            transport->lastError = CAN_TRANSPORT_ERROR_RX_QUEUE_FULL;
        }
    }
}

static void CanTransport_ProcessTx(CanTransport *transport)
{
    CanFrame frame;

    if (transport == NULL)
        return;

    if (transport->txInFlight != 0U)
    {
        if (CanHw_IsTxBusy(&transport->hw) != 0U)
            return;

        CanTransport_OnTxComplete(transport);
    }

    if (transport->txCount == 0U)
        return;

    if (CanTransport_TxPeek(transport, &frame) == 0U)
        return;

    if (CanHw_StartTx(&transport->hw, &frame) == 0U)
    {
        transport->txRetryCount++;
        transport->lastError = CAN_TRANSPORT_ERROR_HW_TX_FAIL;
        return;
    }

    transport->currentTxFrame = frame;
    transport->hwTxOkCountSeen = transport->hw.txOkCount;
    transport->hwTxErrorCountSeen = transport->hw.txErrorCount;
    transport->txInFlight = 1U;
    transport->txStartCount++;
}

uint8_t CanTransport_Init(CanTransport *transport,
                          const CanTransportConfig *config)
{
    CanHwConfig hwConfig;

    if (transport == NULL || config == NULL)
        return 0U;

    (void)memset(transport, 0, sizeof(*transport));

    hwConfig.localNodeId = config->localNodeId;
    hwConfig.instance = config->instance;
    hwConfig.txMbIndex = config->txMbIndex;
    hwConfig.rxMbIndex = config->rxMbIndex;
    hwConfig.driverState = config->driverState;
    hwConfig.userConfig = config->userConfig;

    if (CanHw_Init(&transport->hw, &hwConfig) == 0U)
        return 0U;

    transport->lastError = CAN_TRANSPORT_ERROR_NONE;
    transport->initialized = 1U;
    return 1U;
}

void CanTransport_Task(CanTransport *transport, uint32_t nowMs)
{
    if (transport == NULL || transport->initialized == 0U)
        return;

    CanHw_Task(&transport->hw, nowMs);
    CanTransport_DrainHwRx(transport);
    CanTransport_ProcessTx(transport);
}

uint8_t CanTransport_SendFrame(CanTransport *transport,
                               const CanFrame *frame)
{
    if (transport == NULL || frame == NULL || transport->initialized == 0U)
    {
        if (transport != NULL)
            transport->lastError = CAN_TRANSPORT_ERROR_NOT_READY;
        return 0U;
    }

    if (CanTransport_TxIsFull(transport) != 0U)
    {
        transport->txDropCount++;
        transport->lastError = CAN_TRANSPORT_ERROR_TX_QUEUE_FULL;
        return 0U;
    }

    transport->txQueue[transport->txTail] = *frame;
    transport->txTail = CanTransport_TxNext(transport->txTail);
    transport->txCount++;
    transport->txQueuedCount++;
    return 1U;
}

uint8_t CanTransport_PopRx(CanTransport *transport,
                           CanFrame *outFrame)
{
    if (transport == NULL || outFrame == NULL || transport->initialized == 0U)
        return 0U;

    if (transport->rxCount == 0U)
        return 0U;

    *outFrame = transport->rxQueue[transport->rxHead];
    CanTransport_ClearFrame(&transport->rxQueue[transport->rxHead]);
    transport->rxHead = CanTransport_RxNext(transport->rxHead);
    transport->rxCount--;
    return 1U;
}

uint8_t CanTransport_IsReady(const CanTransport *transport)
{
    if (transport == NULL)
        return 0U;

    return transport->initialized;
}

uint8_t CanTransport_GetLastError(const CanTransport *transport)
{
    if (transport == NULL)
        return CAN_TRANSPORT_ERROR_NOT_READY;

    return transport->lastError;
}
