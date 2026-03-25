#include <string.h>
#include "drivers/drv_lin_slave.h"

EmbResult DrvLinSlave_Init(DrvLinSlave *lin, const DrvLinSlaveConfig *config)
{
    if (lin == 0 || config == 0)
        return EMB_EINVAL;

    (void)memset(lin, 0, sizeof(*lin));
    lin->config = *config;
    lin->state = DRV_LIN_SLAVE_IDLE;
    return EMB_OK;
}

EmbResult DrvLinSlave_Start(DrvLinSlave *lin, HalS32kLinCallback callback)
{
    if (lin == 0 || callback == 0)
        return EMB_EINVAL;

    if (HalS32kLin_Init(&lin->config.port) != EMB_OK)
        return EMB_EIO;
    if (HalS32kLin_InstallCallback(&lin->config.port, callback) != EMB_OK)
        return EMB_EIO;

    return EMB_OK;
}

EmbResult DrvLinSlave_SendResponse(DrvLinSlave *lin, const uint8_t *data, uint8_t size)
{
    if (lin == 0 || data == 0 || size == 0U || size > sizeof(lin->txBuffer))
        return EMB_EINVAL;

    (void)memcpy(lin->txBuffer, data, size);
    if (HalS32kLin_Send(&lin->config.port, lin->txBuffer, size) != EMB_OK)
        return EMB_EIO;

    lin->state = DRV_LIN_SLAVE_WAIT_TX;
    return EMB_OK;
}

EmbResult DrvLinSlave_StartReceive(DrvLinSlave *lin, uint8_t size)
{
    if (lin == 0 || size == 0U || size > sizeof(lin->rxBuffer))
        return EMB_EINVAL;

    if (HalS32kLin_Receive(&lin->config.port, lin->rxBuffer, size) != EMB_OK)
        return EMB_EIO;

    lin->state = DRV_LIN_SLAVE_WAIT_RX;
    return EMB_OK;
}

EmbResult DrvLinSlave_GoIdle(DrvLinSlave *lin)
{
    if (lin == 0)
        return EMB_EINVAL;

    if (HalS32kLin_GoIdle(&lin->config.port) != EMB_OK)
        return EMB_EIO;

    lin->state = DRV_LIN_SLAVE_IDLE;
    return EMB_OK;
}

uint8_t DrvLinSlave_TakeRxFramePending(DrvLinSlave *lin)
{
    EmbCriticalState criticalState = 0U;
    uint8_t pending;

    if (lin == 0)
        return 0U;

    EMB_ENTER_CRITICAL(criticalState);
    pending = lin->rxFramePending;
    lin->rxFramePending = 0U;
    EMB_EXIT_CRITICAL(criticalState);
    return pending;
}

uint8_t DrvLinSlave_TakeTxDonePending(DrvLinSlave *lin)
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

uint8_t DrvLinSlave_TakeTimeoutPending(DrvLinSlave *lin)
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

uint8_t DrvLinSlave_TakeErrorPending(DrvLinSlave *lin, uint8_t *outEventId)
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
