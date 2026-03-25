/*
 * FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다.
 * IsoSdk 계층이 실제 generated driver 호출을 맡고,
 * 위쪽에는 더 단순한 push/pop transport 표면만 노출한다.
 */
#include "can_hw.h"

#include <stddef.h>
#include <string.h>

#include "interrupt_manager.h"

#include "../platform/s32k_sdk/isosdk_can.h"

static CanHw *s_can_hw_instance;

static void CanHw_ClearFrame(CanFrame *frame)
{
    if (frame != NULL)
    {
        (void)memset(frame, 0, sizeof(*frame));
    }
}

static uint8_t CanHw_NextIndex(uint8_t index, uint8_t capacity)
{
    index++;
    if (index >= capacity)
    {
        index = 0U;
    }

    return index;
}

static uint8_t CanHw_RxQueuePush(CanHw *hw, const CanFrame *frame)
{
    if ((hw == NULL) || (frame == NULL))
    {
        return 0U;
    }

    if (hw->rx_count >= CAN_HW_RX_QUEUE_SIZE)
    {
        return 0U;
    }

    hw->rx_queue[hw->rx_tail] = *frame;
    hw->rx_tail = CanHw_NextIndex(hw->rx_tail, CAN_HW_RX_QUEUE_SIZE);
    hw->rx_count++;
    return 1U;
}

static uint8_t CanHw_CopyIsoSdkRxToFrame(uint32_t now_ms, CanFrame *out_frame)
{
    uint8_t dlc;

    if (out_frame == NULL)
    {
        return 0U;
    }

    CanHw_ClearFrame(out_frame);
    dlc = 0U;
    if (IsoSdk_CanReadRxFrame(now_ms,
                              &out_frame->id,
                              &dlc,
                              &out_frame->is_extended_id,
                              &out_frame->is_remote_frame,
                              out_frame->data,
                              CAN_FRAME_DATA_SIZE) == 0U)
    {
        return 0U;
    }

    out_frame->dlc = dlc;
    out_frame->timestamp_ms = now_ms;
    return 1U;
}

static uint8_t CanHw_StartReceive(CanHw *hw)
{
    if (hw == NULL)
    {
        return 0U;
    }

    return IsoSdk_CanStartReceive(hw->instance, hw->rx_mb_index);
}

static void CanHw_OnRxComplete(CanHw *hw)
{
    CanFrame frame;

    if ((hw == NULL) || (hw->initialized == 0U))
    {
        return;
    }

    if (CanHw_CopyIsoSdkRxToFrame(0U, &frame) != 0U)
    {
        if (CanHw_RxQueuePush(hw, &frame) != 0U)
        {
            hw->rx_ok_count++;
        }
        else
        {
            hw->rx_drop_count++;
            hw->last_error = CAN_HW_ERROR_RX_QUEUE_FULL;
        }
    }
    else
    {
        hw->rx_error_count++;
        hw->last_error = CAN_HW_ERROR_RX_STATUS_FAIL;
    }

    if (CanHw_StartReceive(hw) == 0U)
    {
        hw->rx_error_count++;
        hw->last_error = CAN_HW_ERROR_RX_RESTART_FAIL;
    }
}

static void CanHw_OnIsoSdkEvent(void *context, uint8_t event_id, uint8_t mb_index)
{
    CanHw *hw;

    hw = (context != NULL) ? (CanHw *)context : s_can_hw_instance;
    if ((hw == NULL) || (hw->initialized == 0U))
    {
        return;
    }

    if ((event_id == ISOSDK_CAN_EVENT_RX_DONE) && (mb_index == hw->rx_mb_index))
    {
        CanHw_OnRxComplete(hw);
        return;
    }

    if (event_id == ISOSDK_CAN_EVENT_ERROR)
    {
        hw->last_error = CAN_HW_ERROR_RX_STATUS_FAIL;
    }
}

uint8_t CanHw_InitDefault(CanHw *hw, uint8_t local_node_id)
{
    if (hw == NULL)
    {
        return 0U;
    }

    (void)memset(hw, 0, sizeof(*hw));
    hw->local_node_id = local_node_id;
    hw->tx_mb_index = CAN_HW_TX_MB_INDEX;
    hw->rx_mb_index = CAN_HW_RX_MB_INDEX;
    hw->last_error = CAN_HW_ERROR_NONE;
    s_can_hw_instance = hw;

    if (IsoSdk_CanIsSupported() == 0U)
    {
        hw->last_error = CAN_HW_ERROR_INIT_FAIL;
        return 0U;
    }

    hw->instance = IsoSdk_CanGetDefaultInstance();
    if (IsoSdk_CanInitController(hw->instance) == 0U)
    {
        hw->last_error = CAN_HW_ERROR_INIT_FAIL;
        return 0U;
    }

    IsoSdk_CanInstallEventCallback(CanHw_OnIsoSdkEvent, hw);

    if (IsoSdk_CanInitTxMailbox(hw->instance, hw->tx_mb_index) == 0U)
    {
        hw->last_error = CAN_HW_ERROR_TX_CONFIG_FAIL;
        return 0U;
    }

    if (IsoSdk_CanConfigRxAcceptAll(hw->instance, hw->rx_mb_index) == 0U)
    {
        hw->last_error = CAN_HW_ERROR_RX_CONFIG_FAIL;
        return 0U;
    }

    if (IsoSdk_CanInitRxMailbox(hw->instance, hw->rx_mb_index) == 0U)
    {
        hw->last_error = CAN_HW_ERROR_RX_CONFIG_FAIL;
        return 0U;
    }

    hw->initialized = 1U;
    if (CanHw_StartReceive(hw) == 0U)
    {
        hw->initialized = 0U;
        hw->last_error = CAN_HW_ERROR_RX_RESTART_FAIL;
        return 0U;
    }

    return 1U;
}

void CanHw_Task(CanHw *hw, uint32_t now_ms)
{
    IsoSdkCanTransferState transfer_state;

    (void)now_ms;

    if ((hw == NULL) || (hw->initialized == 0U))
    {
        return;
    }

    if (hw->tx_busy != 0U)
    {
        transfer_state = IsoSdk_CanGetTransferState(hw->instance, hw->tx_mb_index);
        if (transfer_state == ISOSDK_CAN_TRANSFER_DONE)
        {
            hw->tx_busy = 0U;
            hw->tx_ok_count++;
        }
        else if (transfer_state == ISOSDK_CAN_TRANSFER_ERROR)
        {
            hw->tx_busy = 0U;
            hw->tx_error_count++;
            hw->last_error = CAN_HW_ERROR_TX_STATUS_FAIL;
        }
    }

    transfer_state = IsoSdk_CanGetTransferState(hw->instance, hw->rx_mb_index);
    if (transfer_state == ISOSDK_CAN_TRANSFER_ERROR)
    {
        hw->rx_error_count++;
        hw->last_error = CAN_HW_ERROR_RX_STATUS_FAIL;
        if (CanHw_StartReceive(hw) == 0U)
        {
            hw->last_error = CAN_HW_ERROR_RX_RESTART_FAIL;
        }
    }
}

uint8_t CanHw_StartTx(CanHw *hw, const CanFrame *frame)
{
    uint8_t dlc;

    if ((hw == NULL) || (frame == NULL) || (hw->initialized == 0U))
    {
        if (hw != NULL)
        {
            hw->last_error = CAN_HW_ERROR_NOT_READY;
        }
        return 0U;
    }

    if (hw->tx_busy != 0U)
    {
        hw->last_error = CAN_HW_ERROR_TX_START_FAIL;
        return 0U;
    }

    dlc = frame->dlc;
    if (dlc > CAN_FRAME_DATA_SIZE)
    {
        dlc = CAN_FRAME_DATA_SIZE;
    }

    if (IsoSdk_CanSend(hw->instance,
                       hw->tx_mb_index,
                       frame->id,
                       dlc,
                       frame->data,
                       frame->is_extended_id,
                       frame->is_remote_frame) == 0U)
    {
        hw->tx_error_count++;
        hw->last_error = CAN_HW_ERROR_TX_START_FAIL;
        return 0U;
    }

    hw->tx_busy = 1U;
    return 1U;
}

uint8_t CanHw_TryPopRx(CanHw *hw, CanFrame *out_frame)
{
    if ((hw == NULL) || (out_frame == NULL) || (hw->initialized == 0U))
    {
        return 0U;
    }

    INT_SYS_DisableIRQGlobal();
    if (hw->rx_count == 0U)
    {
        INT_SYS_EnableIRQGlobal();
        return 0U;
    }

    *out_frame = hw->rx_queue[hw->rx_head];
    CanHw_ClearFrame(&hw->rx_queue[hw->rx_head]);
    hw->rx_head = CanHw_NextIndex(hw->rx_head, CAN_HW_RX_QUEUE_SIZE);
    hw->rx_count--;
    INT_SYS_EnableIRQGlobal();
    return 1U;
}

uint8_t CanHw_IsReady(const CanHw *hw)
{
    return (hw != NULL) ? hw->initialized : 0U;
}

uint8_t CanHw_IsTxBusy(const CanHw *hw)
{
    return (hw != NULL) ? hw->tx_busy : 0U;
}

uint8_t CanHw_GetLastError(const CanHw *hw)
{
    if (hw == NULL)
    {
        return CAN_HW_ERROR_NOT_READY;
    }

    return hw->last_error;
}
