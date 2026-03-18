#include "can_hw.h"

#include <stddef.h>
#include <string.h>

#define CAN_HW_DEFAULT_INSTANCE     INST_FLEXCAN_CONFIG_1

static void CanHw_ClearFrame(CanFrame *frame)
{
    if (frame == NULL)
        return;

    (void)memset(frame, 0, sizeof(*frame));
}

static uint8_t CanHw_RxQueueNext(uint8_t index)
{
    index++;
    if (index >= CAN_HW_RX_QUEUE_SIZE)
        index = 0U;
    return index;
}

static uint8_t CanHw_RxQueueIsFull(const CanHw *hw)
{
    if (hw == NULL)
        return 1U;

    return (hw->rxCount >= CAN_HW_RX_QUEUE_SIZE) ? 1U : 0U;
}

static uint8_t CanHw_RxQueuePush(CanHw *hw, const CanFrame *frame)
{
    if (hw == NULL || frame == NULL)
        return 0U;

    if (CanHw_RxQueueIsFull(hw) != 0U)
        return 0U;

    hw->rxQueue[hw->rxTail] = *frame;
    hw->rxTail = CanHw_RxQueueNext(hw->rxTail);
    hw->rxCount++;
    return 1U;
}

static void CanHw_CopySdkRxToFrame(const flexcan_msgbuff_t *rxMsg,
                                   uint32_t nowMs,
                                   CanFrame *outFrame)
{
    uint8_t dlc;

    if (rxMsg == NULL || outFrame == NULL)
        return;

    CanHw_ClearFrame(outFrame);

    dlc = rxMsg->dataLen;
    if (dlc > CAN_FRAME_DATA_SIZE)
        dlc = CAN_FRAME_DATA_SIZE;

    outFrame->id = rxMsg->msgId;
    outFrame->dlc = dlc;
    outFrame->isExtendedId = 0U;
    outFrame->isRemoteFrame = 0U;
    outFrame->timestampMs = nowMs;

    (void)memcpy(outFrame->data, rxMsg->data, dlc);
}

static void CanHw_FillDataInfo(const CanFrame *frame,
                               flexcan_data_info_t *outInfo)
{
    if (frame == NULL || outInfo == NULL)
        return;

    (void)memset(outInfo, 0, sizeof(*outInfo));
    outInfo->msg_id_type = (frame->isExtendedId != 0U) ?
                           FLEXCAN_MSG_ID_EXT : FLEXCAN_MSG_ID_STD;
    outInfo->is_remote = (frame->isRemoteFrame != 0U) ? true : false;
    outInfo->data_length = frame->dlc;
    outInfo->fd_enable = true;
    outInfo->enable_brs = true;
    outInfo->fd_padding = 0U;
}

static uint8_t CanHw_StartReceive(CanHw *hw)
{
    status_t status;

    if (hw == NULL)
        return 0U;

    (void)memset(&hw->rxMsg, 0, sizeof(hw->rxMsg));

    status = FLEXCAN_DRV_Receive(hw->instance, hw->rxMbIndex, &hw->rxMsg);
    return (status == STATUS_SUCCESS) ? 1U : 0U;
}

static uint8_t CanHw_ConfigAcceptAll(CanHw *hw)
{
    status_t status;

    if (hw == NULL)
        return 0U;

    FLEXCAN_DRV_SetRxMaskType(hw->instance, FLEXCAN_RX_MASK_INDIVIDUAL);
    status = FLEXCAN_DRV_SetRxIndividualMask(hw->instance,
                                             FLEXCAN_MSG_ID_STD,
                                             hw->rxMbIndex,
                                             0x00000000U);
    return (status == STATUS_SUCCESS) ? 1U : 0U;
}

uint8_t CanHw_Init(CanHw *hw, const CanHwConfig *config)
{
    flexcan_data_info_t dataInfo;
    status_t status;

    if (hw == NULL || config == NULL)
        return 0U;

    (void)memset(hw, 0, sizeof(*hw));

    hw->localNodeId = config->localNodeId;
    hw->instance = config->instance;
    hw->txMbIndex = config->txMbIndex;
    hw->rxMbIndex = config->rxMbIndex;
    hw->lastError = CAN_HW_ERROR_NONE;
    hw->driverState = config->driverState;
    hw->userConfig = config->userConfig;

    if (hw->instance == 0U)
        hw->instance = CAN_HW_DEFAULT_INSTANCE;

    (void)memset(&dataInfo, 0, sizeof(dataInfo));
    dataInfo.msg_id_type = FLEXCAN_MSG_ID_STD;
    dataInfo.is_remote = false;
    dataInfo.data_length = CAN_FRAME_DATA_SIZE;
    dataInfo.fd_enable = true;
    dataInfo.enable_brs = true;
    dataInfo.fd_padding = 0U;

    if (hw->driverState == NULL || hw->userConfig == NULL)
    {
        hw->lastError = CAN_HW_ERROR_INIT_FAIL;
        return 0U;
    }

    status = FLEXCAN_DRV_Init(hw->instance, hw->driverState, hw->userConfig);
    if (status != STATUS_SUCCESS)
    {
        hw->lastError = CAN_HW_ERROR_INIT_FAIL;
        return 0U;
    }

    status = FLEXCAN_DRV_ConfigTxMb(hw->instance, hw->txMbIndex, &dataInfo, 0U);
    if (status != STATUS_SUCCESS)
    {
        hw->lastError = CAN_HW_ERROR_TX_CONFIG_FAIL;
        return 0U;
    }

    if (CanHw_ConfigAcceptAll(hw) == 0U)
    {
        hw->lastError = CAN_HW_ERROR_RX_CONFIG_FAIL;
        return 0U;
    }

    status = FLEXCAN_DRV_ConfigRxMb(hw->instance, hw->rxMbIndex, &dataInfo, 0U);
    if (status != STATUS_SUCCESS)
    {
        hw->lastError = CAN_HW_ERROR_RX_CONFIG_FAIL;
        return 0U;
    }

    if (CanHw_StartReceive(hw) == 0U)
    {
        hw->lastError = CAN_HW_ERROR_RX_RESTART_FAIL;
        return 0U;
    }

    hw->initialized = 1U;
    return 1U;
}

void CanHw_Task(CanHw *hw, uint32_t nowMs)
{
    status_t status;
    CanFrame frame;

    if (hw == NULL || hw->initialized == 0U)
        return;

    if (hw->txBusy != 0U)
    {
        status = FLEXCAN_DRV_GetTransferStatus(hw->instance, hw->txMbIndex);
        if (status == STATUS_SUCCESS)
        {
            hw->txBusy = 0U;
            hw->txOkCount++;
        }
        else if (status != STATUS_BUSY)
        {
            hw->txBusy = 0U;
            hw->txErrorCount++;
            hw->lastError = CAN_HW_ERROR_TX_STATUS_FAIL;
        }
    }

    status = FLEXCAN_DRV_GetTransferStatus(hw->instance, hw->rxMbIndex);
    if (status == STATUS_SUCCESS)
    {
        CanHw_CopySdkRxToFrame(&hw->rxMsg, nowMs, &frame);

        if (CanHw_RxQueuePush(hw, &frame) != 0U)
        {
            hw->rxOkCount++;
        }
        else
        {
            hw->rxDropCount++;
            hw->lastError = CAN_HW_ERROR_RX_QUEUE_FULL;
        }

        if (CanHw_StartReceive(hw) == 0U)
        {
            hw->rxErrorCount++;
            hw->lastError = CAN_HW_ERROR_RX_RESTART_FAIL;
        }
    }
    else if (status != STATUS_BUSY)
    {
        hw->rxErrorCount++;
        hw->lastError = CAN_HW_ERROR_RX_STATUS_FAIL;

        if (CanHw_StartReceive(hw) == 0U)
            hw->lastError = CAN_HW_ERROR_RX_RESTART_FAIL;
    }
}

uint8_t CanHw_StartTx(CanHw *hw, const CanFrame *frame)
{
    flexcan_data_info_t dataInfo;
    status_t status;
    uint8_t dlc;

    if (hw == NULL || frame == NULL || hw->initialized == 0U)
    {
        if (hw != NULL)
            hw->lastError = CAN_HW_ERROR_NOT_READY;
        return 0U;
    }

    if (hw->txBusy != 0U)
    {
        hw->lastError = CAN_HW_ERROR_TX_START_FAIL;
        return 0U;
    }

    dlc = frame->dlc;
    if (dlc > CAN_FRAME_DATA_SIZE)
        dlc = CAN_FRAME_DATA_SIZE;

    CanHw_FillDataInfo(frame, &dataInfo);
    dataInfo.data_length = dlc;

    status = FLEXCAN_DRV_Send(hw->instance,
                              hw->txMbIndex,
                              &dataInfo,
                              frame->id,
                              (const uint8_t *)frame->data);
    if (status != STATUS_SUCCESS)
    {
        hw->txErrorCount++;
        hw->lastError = CAN_HW_ERROR_TX_START_FAIL;
        return 0U;
    }

    hw->txBusy = 1U;
    return 1U;
}

uint8_t CanHw_TryPopRx(CanHw *hw, CanFrame *outFrame)
{
    if (hw == NULL || outFrame == NULL || hw->initialized == 0U)
        return 0U;

    if (hw->rxCount == 0U)
        return 0U;

    *outFrame = hw->rxQueue[hw->rxHead];
    CanHw_ClearFrame(&hw->rxQueue[hw->rxHead]);
    hw->rxHead = CanHw_RxQueueNext(hw->rxHead);
    hw->rxCount--;
    return 1U;
}

uint8_t CanHw_IsReady(const CanHw *hw)
{
    if (hw == NULL)
        return 0U;

    return hw->initialized;
}

uint8_t CanHw_IsTxBusy(const CanHw *hw)
{
    if (hw == NULL)
        return 0U;

    return hw->txBusy;
}

uint8_t CanHw_GetLastError(const CanHw *hw)
{
    if (hw == NULL)
        return CAN_HW_ERROR_NOT_READY;

    return hw->lastError;
}
