/*
 * FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다.
 * generated 하드웨어 driver와의 상호작용은 이 파일이 맡고,
 * 위쪽에는 더 단순한 push/pop transport 표면만 노출한다.
 */
#include "can_hw.h"

#include <stddef.h>
#include <string.h>

#include "sdk_project_config.h"

#ifdef INST_FLEXCAN_CONFIG_1

/*
 * raw CAN 계층이 사용하는 공용 SDK RX mailbox 버퍼다.
 * frame 수신이 끝나면 소프트웨어 queue로 복사하여,
 * 상위 계층이 driver 소유 메모리에 직접 의존하지 않게 한다.
 */
static flexcan_msgbuff_t g_can_hw_rx_msg;

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

static void CanHw_CopySdkRxToFrame(const flexcan_msgbuff_t *rx_msg,
                                   uint32_t now_ms,
                                   CanFrame *out_frame)
{
    uint8_t dlc;

    if ((rx_msg == NULL) || (out_frame == NULL))
    {
        return;
    }

    CanHw_ClearFrame(out_frame);
    dlc = rx_msg->dataLen;
    if (dlc > CAN_FRAME_DATA_SIZE)
    {
        dlc = CAN_FRAME_DATA_SIZE;
    }

    out_frame->id = rx_msg->msgId;
    out_frame->dlc = dlc;
    out_frame->is_extended_id = 0U;
    out_frame->is_remote_frame = 0U;
    out_frame->timestamp_ms = now_ms;
    (void)memcpy(out_frame->data, rx_msg->data, dlc);
}

static uint8_t CanHw_StartReceive(CanHw *hw)
{
    status_t status;

    if (hw == NULL)
    {
        return 0U;
    }

    (void)memset(&g_can_hw_rx_msg, 0, sizeof(g_can_hw_rx_msg));
    status = FLEXCAN_DRV_Receive(hw->instance, hw->rx_mb_index, &g_can_hw_rx_msg);
    return (status == STATUS_SUCCESS) ? 1U : 0U;
}

static uint8_t CanHw_ConfigAcceptAll(CanHw *hw)
{
    status_t status;

    if (hw == NULL)
    {
        return 0U;
    }

    FLEXCAN_DRV_SetRxMaskType(hw->instance, FLEXCAN_RX_MASK_INDIVIDUAL);
    status = FLEXCAN_DRV_SetRxIndividualMask(hw->instance,
                                             FLEXCAN_MSG_ID_STD,
                                             hw->rx_mb_index,
                                             0x00000000U);
    return (status == STATUS_SUCCESS) ? 1U : 0U;
}

/*
 * generated FlexCAN 인스턴스와 mailbox 배치를 초기화한다.
 * TX/RX mailbox를 여기서 설정해 두어,
 * 이후 transport 로직이 작은 queue 기반 인터페이스만 상대하게 한다.
 */
uint8_t CanHw_InitDefault(CanHw *hw, uint8_t local_node_id)
{
    flexcan_data_info_t data_info;
    status_t            status;

    if (hw == NULL)
    {
        return 0U;
    }

    (void)memset(hw, 0, sizeof(*hw));
    hw->local_node_id = local_node_id;
    hw->instance = INST_FLEXCAN_CONFIG_1;
    hw->tx_mb_index = CAN_HW_TX_MB_INDEX;
    hw->rx_mb_index = CAN_HW_RX_MB_INDEX;
    hw->last_error = CAN_HW_ERROR_NONE;

    (void)memset(&data_info, 0, sizeof(data_info));
    data_info.msg_id_type = FLEXCAN_MSG_ID_STD;
    data_info.is_remote = false;
    data_info.data_length = CAN_FRAME_DATA_SIZE;
    data_info.fd_enable = true;
    data_info.enable_brs = true;
    data_info.fd_padding = 0U;

    /*
     * 사용자 바인딩 확인:
     * 생성된 FlexCAN 주변장치 이름이 바뀌면,
     * 이 파일이 참조하는 바인딩 심볼도 함께 수정해야 한다.
     */
    status = FLEXCAN_DRV_Init(hw->instance, &flexcanState0, &flexcanInitConfig0);
    if (status != STATUS_SUCCESS)
    {
        hw->last_error = CAN_HW_ERROR_INIT_FAIL;
        return 0U;
    }

    status = FLEXCAN_DRV_ConfigTxMb(hw->instance, hw->tx_mb_index, &data_info, 0U);
    if (status != STATUS_SUCCESS)
    {
        hw->last_error = CAN_HW_ERROR_TX_CONFIG_FAIL;
        return 0U;
    }

    if (CanHw_ConfigAcceptAll(hw) == 0U)
    {
        hw->last_error = CAN_HW_ERROR_RX_CONFIG_FAIL;
        return 0U;
    }

    status = FLEXCAN_DRV_ConfigRxMb(hw->instance, hw->rx_mb_index, &data_info, 0U);
    if (status != STATUS_SUCCESS)
    {
        hw->last_error = CAN_HW_ERROR_RX_CONFIG_FAIL;
        return 0U;
    }

    if (CanHw_StartReceive(hw) == 0U)
    {
        hw->last_error = CAN_HW_ERROR_RX_RESTART_FAIL;
        return 0U;
    }

    hw->initialized = 1U;
    return 1U;
}

/*
 * TX/RX 완료 여부를 보기 위해 raw CAN 하드웨어 상태를 poll한다.
 * 끝난 RX frame은 소프트웨어 queue로 옮기고,
 * TX 카운터와 오류 flag는 상위 계층이 볼 수 있게 갱신한다.
 */
void CanHw_Task(CanHw *hw, uint32_t now_ms)
{
    status_t status;
    CanFrame frame;

    if ((hw == NULL) || (hw->initialized == 0U))
    {
        return;
    }

    if (hw->tx_busy != 0U)
    {
        status = FLEXCAN_DRV_GetTransferStatus(hw->instance, hw->tx_mb_index);
        if (status == STATUS_SUCCESS)
        {
            hw->tx_busy = 0U;
            hw->tx_ok_count++;
        }
        else if (status != STATUS_BUSY)
        {
            hw->tx_busy = 0U;
            hw->tx_error_count++;
            hw->last_error = CAN_HW_ERROR_TX_STATUS_FAIL;
        }
    }

    status = FLEXCAN_DRV_GetTransferStatus(hw->instance, hw->rx_mb_index);
    if (status == STATUS_SUCCESS)
    {
        CanHw_CopySdkRxToFrame(&g_can_hw_rx_msg, now_ms, &frame);
        if (CanHw_RxQueuePush(hw, &frame) != 0U)
        {
            hw->rx_ok_count++;
        }
        else
        {
            hw->rx_drop_count++;
            hw->last_error = CAN_HW_ERROR_RX_QUEUE_FULL;
        }

        if (CanHw_StartReceive(hw) == 0U)
        {
            hw->rx_error_count++;
            hw->last_error = CAN_HW_ERROR_RX_RESTART_FAIL;
        }
    }
    else if (status != STATUS_BUSY)
    {
        hw->rx_error_count++;
        hw->last_error = CAN_HW_ERROR_RX_STATUS_FAIL;
        if (CanHw_StartReceive(hw) == 0U)
        {
            hw->last_error = CAN_HW_ERROR_RX_RESTART_FAIL;
        }
    }
}

/*
 * 준비된 raw CAN frame 하나의 전송을 시작한다.
 * 이전 mailbox 전송이 아직 진행 중이면 새 작업을 거부하여,
 * transport 계층이 출력을 직렬화할 수 있게 한다.
 */
uint8_t CanHw_StartTx(CanHw *hw, const CanFrame *frame)
{
    flexcan_data_info_t data_info;
    status_t            status;
    uint8_t             dlc;

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

    (void)memset(&data_info, 0, sizeof(data_info));
    data_info.msg_id_type = (frame->is_extended_id != 0U) ? FLEXCAN_MSG_ID_EXT : FLEXCAN_MSG_ID_STD;
    data_info.is_remote = (frame->is_remote_frame != 0U) ? true : false;
    data_info.data_length = dlc;
    data_info.fd_enable = true;
    data_info.enable_brs = true;
    data_info.fd_padding = 0U;

    status = FLEXCAN_DRV_Send(hw->instance,
                              hw->tx_mb_index,
                              &data_info,
                              frame->id,
                              (const uint8_t *)frame->data);
    if (status != STATUS_SUCCESS)
    {
        hw->tx_error_count++;
        hw->last_error = CAN_HW_ERROR_TX_START_FAIL;
        return 0U;
    }

    hw->tx_busy = 1U;
    return 1U;
}

/*
 * 소프트웨어 queue에서 수신 raw CAN frame 하나를 꺼낸다.
 * transport는 이 queue를 먼저 비운 뒤,
 * 메시지를 service 계층의 논리 수신 경로로 decode한다.
 */
uint8_t CanHw_TryPopRx(CanHw *hw, CanFrame *out_frame)
{
    if ((hw == NULL) || (out_frame == NULL) || (hw->initialized == 0U))
    {
        return 0U;
    }

    if (hw->rx_count == 0U)
    {
        return 0U;
    }

    *out_frame = hw->rx_queue[hw->rx_head];
    CanHw_ClearFrame(&hw->rx_queue[hw->rx_head]);
    hw->rx_head = CanHw_NextIndex(hw->rx_head, CAN_HW_RX_QUEUE_SIZE);
    hw->rx_count--;
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

#else

uint8_t CanHw_InitDefault(CanHw *hw, uint8_t local_node_id)
{
    if (hw != NULL)
    {
        (void)memset(hw, 0, sizeof(*hw));
        hw->local_node_id = local_node_id;
        hw->last_error = CAN_HW_ERROR_INIT_FAIL;
    }

    return 0U;
}

void CanHw_Task(CanHw *hw, uint32_t now_ms)
{
    (void)hw;
    (void)now_ms;
}

uint8_t CanHw_StartTx(CanHw *hw, const CanFrame *frame)
{
    (void)frame;

    if (hw != NULL)
    {
        hw->last_error = CAN_HW_ERROR_NOT_READY;
    }

    return 0U;
}

uint8_t CanHw_TryPopRx(CanHw *hw, CanFrame *out_frame)
{
    (void)hw;
    (void)out_frame;
    return 0U;
}

uint8_t CanHw_IsReady(const CanHw *hw)
{
    (void)hw;
    return 0U;
}

uint8_t CanHw_IsTxBusy(const CanHw *hw)
{
    (void)hw;
    return 0U;
}

uint8_t CanHw_GetLastError(const CanHw *hw)
{
    if (hw == NULL)
    {
        return CAN_HW_ERROR_NOT_READY;
    }

    return hw->last_error;
}

#endif
