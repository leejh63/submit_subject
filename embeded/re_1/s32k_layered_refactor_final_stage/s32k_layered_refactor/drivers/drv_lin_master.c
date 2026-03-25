#include <string.h>
#include "drivers/drv_lin_master.h"

EmbResult DrvLinMaster_Init(DrvLinMaster *lin, const DrvLinMasterConfig *config)
{
    if (lin == 0 || config == 0)
        return EMB_EINVAL;

    (void)memset(lin, 0, sizeof(*lin));
    lin->config = *config;
    lin->state = DRV_LIN_MASTER_IDLE;
    lin->operation = DRV_LIN_MASTER_OP_NONE;
    return EMB_OK;
}

EmbResult DrvLinMaster_Start(DrvLinMaster *lin, HalS32kLinCallback callback)
{
    if (lin == 0 || callback == 0)
        return EMB_EINVAL;

    if (HalS32kLin_Init(&lin->config.port) != EMB_OK)
        return EMB_EIO;
    if (HalS32kLin_InstallCallback(&lin->config.port, callback) != EMB_OK)
        return EMB_EIO;
    if (HalS32kLin_GoIdle(&lin->config.port) != EMB_OK)
        return EMB_EIO;

    lin->state = DRV_LIN_MASTER_IDLE;
    return EMB_OK;
}

EmbResult DrvLinMaster_RequestStatus(DrvLinMaster *lin)
{
    if (lin == 0)
        return EMB_EINVAL;
    if (lin->state != DRV_LIN_MASTER_IDLE)
        return EMB_EBUSY;

    if (HalS32kLin_SetTimeout(&lin->config.port, lin->config.timeoutTicks) != EMB_OK)
        return EMB_EIO;
    if (HalS32kLin_MasterSendHeader(&lin->config.port, lin->config.statusPid) != EMB_OK)
        return EMB_EIO;

    lin->lastPid = lin->config.statusPid;
    lin->operation = DRV_LIN_MASTER_OP_READ_STATUS;
    lin->state = DRV_LIN_MASTER_WAIT_HEADER;
    lin->headerRequestCount++;
    return EMB_OK;
}

EmbResult DrvLinMaster_RequestOkCmd(DrvLinMaster *lin, const uint8_t *data, uint8_t size)
{
    if (lin == 0 || data == 0 || size == 0U || size > sizeof(lin->txBuffer))
        return EMB_EINVAL;
    if (lin->state != DRV_LIN_MASTER_IDLE)
        return EMB_EBUSY;

    (void)memcpy(lin->txBuffer, data, size);
    if (HalS32kLin_SetTimeout(&lin->config.port, lin->config.timeoutTicks) != EMB_OK)
        return EMB_EIO;
    if (HalS32kLin_MasterSendHeader(&lin->config.port, lin->config.okCmdPid) != EMB_OK)
        return EMB_EIO;

    lin->lastPid = lin->config.okCmdPid;
    lin->operation = DRV_LIN_MASTER_OP_SEND_OK_CMD;
    lin->state = DRV_LIN_MASTER_WAIT_HEADER;
    lin->headerRequestCount++;
    return EMB_OK;
}

EmbResult DrvLinMaster_StartStatusReceive(DrvLinMaster *lin)
{
    if (lin == 0)
        return EMB_EINVAL;

    if (HalS32kLin_Receive(&lin->config.port, lin->rxBuffer, lin->config.rxSize) != EMB_OK)
        return EMB_EIO;

    lin->state = DRV_LIN_MASTER_WAIT_RX;
    return EMB_OK;
}

EmbResult DrvLinMaster_StartOkCmdSend(DrvLinMaster *lin)
{
    if (lin == 0)
        return EMB_EINVAL;

    if (HalS32kLin_Send(&lin->config.port, lin->txBuffer, lin->config.txSize) != EMB_OK)
        return EMB_EIO;

    lin->state = DRV_LIN_MASTER_WAIT_TX;
    return EMB_OK;
}

EmbResult DrvLinMaster_GoIdle(DrvLinMaster *lin)
{
    if (lin == 0)
        return EMB_EINVAL;

    if (HalS32kLin_GoIdle(&lin->config.port) != EMB_OK)
        return EMB_EIO;

    lin->state = DRV_LIN_MASTER_IDLE;
    lin->operation = DRV_LIN_MASTER_OP_NONE;
    return EMB_OK;
}

EmbResult DrvLinMaster_TryGetStatusFrame(DrvLinMaster *lin, uint8_t *outData, uint8_t *outSize)
{
    EmbCriticalState criticalState = 0U;

    if (lin == 0 || outData == 0 || outSize == 0)
        return EMB_EINVAL;

    EMB_ENTER_CRITICAL(criticalState);
    if (lin->statusFrameReady == 0U)
    {
        EMB_EXIT_CRITICAL(criticalState);
        return EMB_ESTATE;
    }

    (void)memcpy(outData, lin->rxBuffer, lin->config.rxSize);
    *outSize = lin->config.rxSize;
    lin->statusFrameReady = 0U;
    EMB_EXIT_CRITICAL(criticalState);
    return EMB_OK;
}

void DrvLinMaster_OnCallback(DrvLinMaster *lin, void *linStatePtr)
{
    lin_state_t *linState;

    if (lin == 0 || linStatePtr == 0)
        return;

    linState = (lin_state_t *)linStatePtr;
    lin->callbackCount++;
    lin->lastCallbackEventId = linState->currentEventId;

    if (linState->timeoutCounterFlag != false)
    {
        linState->timeoutCounterFlag = false;
        lin->timeoutPending = 1U;
        lin->timeoutCount++;
        (void)DrvLinMaster_GoIdle(lin);
        return;
    }

    switch (linState->currentEventId)
    {
        case LIN_PID_OK:
            (void)HalS32kLin_SetTimeout(&lin->config.port, lin->config.timeoutTicks);
            if ((lin->operation == DRV_LIN_MASTER_OP_READ_STATUS) &&
                (linState->currentId == lin->config.statusPid))
            {
                if (DrvLinMaster_StartStatusReceive(lin) != EMB_OK)
                {
                    lin->errorPending = 1U;
                    lin->lastErrorEventId = LIN_PID_OK;
                    lin->errorCount++;
                    (void)DrvLinMaster_GoIdle(lin);
                }
            }
            else if ((lin->operation == DRV_LIN_MASTER_OP_SEND_OK_CMD) &&
                     (linState->currentId == lin->config.okCmdPid))
            {
                if (DrvLinMaster_StartOkCmdSend(lin) != EMB_OK)
                {
                    lin->errorPending = 1U;
                    lin->lastErrorEventId = LIN_PID_OK;
                    lin->errorCount++;
                    (void)DrvLinMaster_GoIdle(lin);
                }
            }
            else
            {
                lin->errorPending = 1U;
                lin->lastErrorEventId = LIN_PID_OK;
                lin->errorCount++;
                (void)DrvLinMaster_GoIdle(lin);
            }
            break;

        case LIN_RX_COMPLETED:
            if (lin->operation == DRV_LIN_MASTER_OP_READ_STATUS)
            {
                lin->statusFrameReady = 1U;
                lin->rxCompletedCount++;
            }
            else
            {
                lin->errorPending = 1U;
                lin->lastErrorEventId = LIN_RX_COMPLETED;
                lin->errorCount++;
            }
            (void)DrvLinMaster_GoIdle(lin);
            break;

        case LIN_TX_COMPLETED:
            lin->txDonePending = 1U;
            lin->txCompletedCount++;
            (void)DrvLinMaster_GoIdle(lin);
            break;

        case LIN_RECV_BREAK_FIELD_OK:
            (void)HalS32kLin_SetTimeout(&lin->config.port, lin->config.timeoutTicks);
            break;

        case LIN_PID_ERROR:
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_RX_OVERRUN:
            lin->errorPending = 1U;
            lin->lastErrorEventId = (uint8_t)linState->currentEventId;
            lin->errorCount++;
            (void)DrvLinMaster_GoIdle(lin);
            break;

        default:
            break;
    }
}

uint8_t DrvLinMaster_TakeTxDonePending(DrvLinMaster *lin)
{
    EmbCriticalState criticalState = 0U;
    uint8_t pending;

    if (lin == 0)
        return 0U;

    EMB_ENTER_CRITICAL(criticalState);
    pending = lin->txDonePending;
    lin->txDonePending = 0U;
    EMB_EXIT_CRITICAL(criticalState);
    return pending;
}

uint8_t DrvLinMaster_TakeTimeoutPending(DrvLinMaster *lin)
{
    EmbCriticalState criticalState = 0U;
    uint8_t pending;

    if (lin == 0)
        return 0U;

    EMB_ENTER_CRITICAL(criticalState);
    pending = lin->timeoutPending;
    lin->timeoutPending = 0U;
    EMB_EXIT_CRITICAL(criticalState);
    return pending;
}

uint8_t DrvLinMaster_TakeErrorPending(DrvLinMaster *lin, uint8_t *outEventId)
{
    EmbCriticalState criticalState = 0U;
    uint8_t pending;

    if (lin == 0)
        return 0U;

    EMB_ENTER_CRITICAL(criticalState);
    pending = lin->errorPending;
    if ((pending != 0U) && (outEventId != 0))
        *outEventId = lin->lastErrorEventId;

    lin->errorPending = 0U;
    EMB_EXIT_CRITICAL(criticalState);
    return pending;
}
