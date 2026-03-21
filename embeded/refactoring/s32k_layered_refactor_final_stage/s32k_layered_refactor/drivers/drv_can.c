#include <string.h>
#include "drivers/drv_can.h"

static uint8_t DrvCan_NextIndex(uint8_t index, uint8_t size)
{
    index++;
    if (index >= size)
        index = 0U;
    return index;
}

static EmbResult DrvCan_PushRx(DrvCan *can, const HalCanFrame *frame)
{
    if (can == 0 || frame == 0)
        return EMB_EINVAL;

    if (can->rxCount >= DRV_CAN_RX_QUEUE_SIZE)
    {
        can->rxDropCount++;
        can->lastError = EMB_ENOSPACE;
        return EMB_ENOSPACE;
    }

    can->rxQueue[can->rxTail] = *frame;
    can->rxTail = DrvCan_NextIndex(can->rxTail, DRV_CAN_RX_QUEUE_SIZE);
    can->rxCount++;
    can->rxOkCount++;
    return EMB_OK;
}

EmbResult DrvCan_Init(DrvCan *can, const DrvCanConfig *config)
{
    if (can == 0 || config == 0)
        return EMB_EINVAL;

    (void)memset(can, 0, sizeof(*can));
    can->config = *config;
    can->lastError = EMB_OK;
    return EMB_OK;
}

EmbResult DrvCan_Start(DrvCan *can)
{
    if (can == 0)
        return EMB_EINVAL;

    if (HalS32kCan_Init(&can->config.port) != EMB_OK)
    {
        can->lastError = EMB_EIO;
        return EMB_EIO;
    }

    if (HalS32kCan_Start(&can->config.port) != EMB_OK)
    {
        can->lastError = EMB_EIO;
        return EMB_EIO;
    }

    can->started = 1U;
    return EMB_OK;
}

EmbResult DrvCan_Process(DrvCan *can)
{
    HalCanFrame frame;
    uint8_t txBusy;
    EmbResult status;

    if (can == 0 || can->started == 0U)
        return EMB_ESTATE;

    status = HalS32kCan_TryRead(&can->config.port, &frame);
    if (status == EMB_OK)
    {
        (void)DrvCan_PushRx(can, &frame);
    }
    else if (status != EMB_EBUSY && status != EMB_EUNSUPPORTED)
    {
        can->errorCount++;
        can->lastError = status;
    }

    if (can->txInFlight != 0U)
    {
        status = HalS32kCan_IsTxBusy(&can->config.port, &txBusy);
        if (status == EMB_OK)
        {
            if (txBusy == 0U)
            {
                can->txInFlight = 0U;
                can->txOkCount++;
            }
        }
        else if (status != EMB_EUNSUPPORTED)
        {
            can->errorCount++;
            can->lastError = status;
            can->txInFlight = 0U;
        }
    }

    if (can->txInFlight == 0U && can->txCount > 0U)
    {
        status = HalS32kCan_Tx(&can->config.port, &can->txQueue[can->txHead]);
        if (status == EMB_OK)
        {
            can->txHead = DrvCan_NextIndex(can->txHead, DRV_CAN_TX_QUEUE_SIZE);
            can->txCount--;
            can->txInFlight = 1U;
            can->txStartCount++;
        }
        else if (status != EMB_EUNSUPPORTED)
        {
            can->errorCount++;
            can->lastError = status;
        }
    }

    return EMB_OK;
}

EmbResult DrvCan_Send(DrvCan *can, const HalCanFrame *frame)
{
    if (can == 0 || frame == 0)
        return EMB_EINVAL;
    if (can->started == 0U)
        return EMB_ESTATE;
    if (frame->dlc > 8U)
        return EMB_EINVAL;

    if (can->txCount >= DRV_CAN_TX_QUEUE_SIZE)
    {
        can->txDropCount++;
        can->lastError = EMB_ENOSPACE;
        return EMB_ENOSPACE;
    }

    can->txQueue[can->txTail] = *frame;
    can->txTail = DrvCan_NextIndex(can->txTail, DRV_CAN_TX_QUEUE_SIZE);
    can->txCount++;
    return EMB_OK;
}

EmbResult DrvCan_TryReceive(DrvCan *can, HalCanFrame *frame)
{
    if (can == 0 || frame == 0)
        return EMB_EINVAL;
    if (can->started == 0U)
        return EMB_ESTATE;
    if (can->rxCount == 0U)
        return EMB_EBUSY;

    *frame = can->rxQueue[can->rxHead];
    can->rxHead = DrvCan_NextIndex(can->rxHead, DRV_CAN_RX_QUEUE_SIZE);
    can->rxCount--;
    return EMB_OK;
}
