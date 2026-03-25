#include "isosdk_can.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "isosdk_sdk_bindings.h"

#ifdef ISOSDK_SDK_HAS_CAN

#define ISOSDK_CAN_FRAME_DATA_SIZE  16U
#define ISOSDK_CAN_INVALID_MB_INDEX 0xFFU

static flexcan_msgbuff_t      s_iso_sdk_can_rx_msg;
static IsoSdkCanEventCallback s_iso_sdk_can_event_cb;
static void                  *s_iso_sdk_can_event_context;

static void IsoSdk_CanDispatchEvent(uint8_t event_id, uint8_t mb_index)
{
    if (s_iso_sdk_can_event_cb == NULL)
    {
        return;
    }

    s_iso_sdk_can_event_cb(s_iso_sdk_can_event_context, event_id, mb_index);
}

static void IsoSdk_CanSdkEventCallback(uint8_t instance,
                                       flexcan_event_type_t eventType,
                                       uint32_t buffIdx,
                                       flexcan_state_t *flexcanState)
{
    uint8_t event_id;

    (void)instance;
    (void)flexcanState;

    event_id = ISOSDK_CAN_EVENT_NONE;
    switch (eventType)
    {
        case FLEXCAN_EVENT_RX_COMPLETE:
            event_id = ISOSDK_CAN_EVENT_RX_DONE;
            break;

        case FLEXCAN_EVENT_TX_COMPLETE:
            event_id = ISOSDK_CAN_EVENT_TX_DONE;
            break;

        default:
            break;
    }

    if (event_id != ISOSDK_CAN_EVENT_NONE)
    {
        IsoSdk_CanDispatchEvent(event_id, (uint8_t)buffIdx);
    }
}

static void IsoSdk_CanSdkErrorCallback(uint8_t instance,
                                       flexcan_event_type_t eventType,
                                       flexcan_state_t *flexcanState)
{
    (void)instance;
    (void)eventType;
    (void)flexcanState;
    IsoSdk_CanDispatchEvent(ISOSDK_CAN_EVENT_ERROR, ISOSDK_CAN_INVALID_MB_INDEX);
}

static void IsoSdk_CanInitDataInfo(flexcan_data_info_t *data_info,
                                   uint8_t dlc,
                                   uint8_t is_extended_id,
                                   uint8_t is_remote_frame)
{
    if (data_info == NULL)
    {
        return;
    }

    (void)memset(data_info, 0, sizeof(*data_info));
    data_info->msg_id_type = (is_extended_id != 0U) ? FLEXCAN_MSG_ID_EXT : FLEXCAN_MSG_ID_STD;
    data_info->is_remote = (is_remote_frame != 0U) ? true : false;
    data_info->data_length = dlc;
    data_info->fd_enable = true;
    data_info->enable_brs = true;
    data_info->fd_padding = 0U;
}

uint8_t IsoSdk_CanIsSupported(void)
{
    return 1U;
}

uint8_t IsoSdk_CanGetDefaultInstance(void)
{
    return ISOSDK_SDK_CAN_INSTANCE;
}

uint8_t IsoSdk_CanInitController(uint8_t instance)
{
    return (FLEXCAN_DRV_Init(instance,
                             &ISOSDK_SDK_CAN_STATE,
                             &ISOSDK_SDK_CAN_INIT_CONFIG) == STATUS_SUCCESS) ? 1U : 0U;
}

void IsoSdk_CanInstallEventCallback(IsoSdkCanEventCallback event_cb,
                                    void *event_context)
{
    s_iso_sdk_can_event_cb = event_cb;
    s_iso_sdk_can_event_context = event_context;

    FLEXCAN_DRV_InstallEventCallback(ISOSDK_SDK_CAN_INSTANCE,
                                     (event_cb != NULL) ? IsoSdk_CanSdkEventCallback : NULL,
                                     NULL);
    FLEXCAN_DRV_InstallErrorCallback(ISOSDK_SDK_CAN_INSTANCE,
                                     (event_cb != NULL) ? IsoSdk_CanSdkErrorCallback : NULL,
                                     NULL);
}

uint8_t IsoSdk_CanInitTxMailbox(uint8_t instance, uint8_t tx_mb_index)
{
    flexcan_data_info_t data_info;

    IsoSdk_CanInitDataInfo(&data_info, ISOSDK_CAN_FRAME_DATA_SIZE, 0U, 0U);
    return (FLEXCAN_DRV_ConfigTxMb(instance, tx_mb_index, &data_info, 0U) == STATUS_SUCCESS) ? 1U : 0U;
}

uint8_t IsoSdk_CanInitRxMailbox(uint8_t instance, uint8_t rx_mb_index)
{
    flexcan_data_info_t data_info;

    IsoSdk_CanInitDataInfo(&data_info, ISOSDK_CAN_FRAME_DATA_SIZE, 0U, 0U);
    return (FLEXCAN_DRV_ConfigRxMb(instance, rx_mb_index, &data_info, 0U) == STATUS_SUCCESS) ? 1U : 0U;
}

uint8_t IsoSdk_CanConfigRxAcceptAll(uint8_t instance, uint8_t rx_mb_index)
{
    status_t status;

    FLEXCAN_DRV_SetRxMaskType(instance, FLEXCAN_RX_MASK_INDIVIDUAL);
    status = FLEXCAN_DRV_SetRxIndividualMask(instance,
                                             FLEXCAN_MSG_ID_STD,
                                             rx_mb_index,
                                             0x00000000U);
    return (status == STATUS_SUCCESS) ? 1U : 0U;
}

uint8_t IsoSdk_CanStartReceive(uint8_t instance, uint8_t rx_mb_index)
{
    (void)memset(&s_iso_sdk_can_rx_msg, 0, sizeof(s_iso_sdk_can_rx_msg));
    return (FLEXCAN_DRV_Receive(instance, rx_mb_index, &s_iso_sdk_can_rx_msg) == STATUS_SUCCESS) ? 1U : 0U;
}

IsoSdkCanTransferState IsoSdk_CanGetTransferState(uint8_t instance, uint8_t mb_index)
{
    status_t status;

    status = FLEXCAN_DRV_GetTransferStatus(instance, mb_index);
    if (status == STATUS_SUCCESS)
    {
        return ISOSDK_CAN_TRANSFER_DONE;
    }

    if (status == STATUS_BUSY)
    {
        return ISOSDK_CAN_TRANSFER_BUSY;
    }

    return ISOSDK_CAN_TRANSFER_ERROR;
}

uint8_t IsoSdk_CanReadRxFrame(uint32_t now_ms,
                              uint32_t *out_id,
                              uint8_t *out_dlc,
                              uint8_t *out_is_extended_id,
                              uint8_t *out_is_remote_frame,
                              uint8_t *out_data,
                              uint8_t data_capacity)
{
    uint8_t dlc;

    if ((out_id == NULL) || (out_dlc == NULL) ||
        (out_is_extended_id == NULL) || (out_is_remote_frame == NULL) ||
        (out_data == NULL) || (data_capacity == 0U))
    {
        return 0U;
    }

    dlc = s_iso_sdk_can_rx_msg.dataLen;
    if (dlc > data_capacity)
    {
        dlc = data_capacity;
    }

    *out_id = s_iso_sdk_can_rx_msg.msgId;
    *out_dlc = dlc;
    *out_is_extended_id = 0U;
    *out_is_remote_frame = 0U;
    (void)now_ms;
    (void)memcpy(out_data, s_iso_sdk_can_rx_msg.data, dlc);
    return 1U;
}

uint8_t IsoSdk_CanSend(uint8_t instance,
                       uint8_t tx_mb_index,
                       uint32_t id,
                       uint8_t dlc,
                       const uint8_t *data,
                       uint8_t is_extended_id,
                       uint8_t is_remote_frame)
{
    flexcan_data_info_t data_info;

    if (data == NULL)
    {
        return 0U;
    }

    IsoSdk_CanInitDataInfo(&data_info, dlc, is_extended_id, is_remote_frame);
    return (FLEXCAN_DRV_Send(instance,
                             tx_mb_index,
                             &data_info,
                             id,
                             data) == STATUS_SUCCESS) ? 1U : 0U;
}

#else

uint8_t IsoSdk_CanIsSupported(void)
{
    return 0U;
}

uint8_t IsoSdk_CanGetDefaultInstance(void)
{
    return 0U;
}

uint8_t IsoSdk_CanInitController(uint8_t instance)
{
    (void)instance;
    return 0U;
}

void IsoSdk_CanInstallEventCallback(IsoSdkCanEventCallback event_cb,
                                    void *event_context)
{
    (void)event_cb;
    (void)event_context;
}

uint8_t IsoSdk_CanInitTxMailbox(uint8_t instance, uint8_t tx_mb_index)
{
    (void)instance;
    (void)tx_mb_index;
    return 0U;
}

uint8_t IsoSdk_CanInitRxMailbox(uint8_t instance, uint8_t rx_mb_index)
{
    (void)instance;
    (void)rx_mb_index;
    return 0U;
}

uint8_t IsoSdk_CanConfigRxAcceptAll(uint8_t instance, uint8_t rx_mb_index)
{
    (void)instance;
    (void)rx_mb_index;
    return 0U;
}

uint8_t IsoSdk_CanStartReceive(uint8_t instance, uint8_t rx_mb_index)
{
    (void)instance;
    (void)rx_mb_index;
    return 0U;
}

IsoSdkCanTransferState IsoSdk_CanGetTransferState(uint8_t instance, uint8_t mb_index)
{
    (void)instance;
    (void)mb_index;
    return ISOSDK_CAN_TRANSFER_ERROR;
}

uint8_t IsoSdk_CanReadRxFrame(uint32_t now_ms,
                              uint32_t *out_id,
                              uint8_t *out_dlc,
                              uint8_t *out_is_extended_id,
                              uint8_t *out_is_remote_frame,
                              uint8_t *out_data,
                              uint8_t data_capacity)
{
    (void)now_ms;
    (void)out_id;
    (void)out_dlc;
    (void)out_is_extended_id;
    (void)out_is_remote_frame;
    (void)out_data;
    (void)data_capacity;
    return 0U;
}

uint8_t IsoSdk_CanSend(uint8_t instance,
                       uint8_t tx_mb_index,
                       uint32_t id,
                       uint8_t dlc,
                       const uint8_t *data,
                       uint8_t is_extended_id,
                       uint8_t is_remote_frame)
{
    (void)instance;
    (void)tx_mb_index;
    (void)id;
    (void)dlc;
    (void)data;
    (void)is_extended_id;
    (void)is_remote_frame;
    return 0U;
}

#endif
