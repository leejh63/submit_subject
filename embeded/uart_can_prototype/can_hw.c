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
    // 일단 임시로 데이터 크기 제한 // 주변상황에 맞춰서 변경 해야함
    if (dlc > CAN_FRAME_DATA_SIZE)
        dlc = CAN_FRAME_DATA_SIZE;
// 헤더 복사
    outFrame->id = rxMsg->msgId;
    outFrame->dlc = dlc;
    outFrame->isExtendedId = 0U;
    outFrame->isRemoteFrame = 0U;
    outFrame->timestampMs = nowMs;
// 데이터 복사
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
    // 이걸 조절하는 걸 통해서 어떤 인덱스, 어떤 버퍼를 쓸지  결정가능 
    // 그렇다면 큰 분류를 여기서 미리하고 
    // 소프트웨어적으로 분류하는것도 방법일듯 
    // 모듈이 이걸 처리하니 cpu부담도 적을듯?
    status = FLEXCAN_DRV_Receive(hw->instance, hw->rxMbIndex, &hw->rxMsg);
    return (status == STATUS_SUCCESS) ? 1U : 0U;
}

static uint8_t CanHw_ConfigAcceptAll(CanHw *hw)
{
    status_t status;

    if (hw == NULL)
        return 0U;

    FLEXCAN_DRV_SetRxMaskType(hw->instance, FLEXCAN_RX_MASK_INDIVIDUAL);
    // 이게 마스크인데 넷마스킹과 비스무리하게 작동하는듯?
    // 모듈을 추가할때 이미 존재하는 모듈과 하드웨어 단에서 처리하도록 하는것도 좋을듯?
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
    // tx메세지 버퍼 등록
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
    // rx메세지 버퍼 등록
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
// 현재는 풀링 방식으로 하나 하나 받아오기 때문에 맘에 들진 않음
// 인터럽트 핸들러/콜백 기반 CAN 수신 형태로 rx/tx 수신을 transport r/tx 큐에 넣는 방향으로 변경하는게 좀더 좋은것같은뎅
// 혹은 가능하다면 dma 형태가 좋음 아마 예제코드가 dma인것 같아보였음 
// 하지만 수신 모델 자체를 전부 뜯어 고쳐야 될수도 있음 
// 일단은 폴링 방식으로 레이어 나눈것 과 인터럽트 형태로 변경후 코드 수정량을 보고 진행할 생각 
    if (hw->txBusy != 0U)
    {   // tx가 작동 중일때  Status를 가져와서 적절한 값 세팅
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
// RX는 외부에서 임의의 시점에 들어오므로 항상 확인
    status = FLEXCAN_DRV_GetTransferStatus(hw->instance, hw->rxMbIndex);
    if (status == STATUS_SUCCESS)
    {
        // rxMsg여기에 담긴 메세지를 frame여기로 옮김
        CanHw_CopySdkRxToFrame(&hw->rxMsg, nowMs, &frame);
        // 수신 큐에 프레임 담음
        if (CanHw_RxQueuePush(hw, &frame) != 0U)
        {
            hw->rxOkCount++;
        }
        else
        {
            hw->rxDropCount++;
            hw->lastError = CAN_HW_ERROR_RX_QUEUE_FULL;
        }
        // 다시 캔통신 수신 세팅
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
        // 다시 캔통신 수신 세팅
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
