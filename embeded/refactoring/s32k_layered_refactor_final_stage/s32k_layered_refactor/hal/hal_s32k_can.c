#include <string.h>
#include "platform/s32k_bindings.h"
#include "hal/hal_s32k_can.h"

#if defined(S32K_MASTER_CAN_INSTANCE) || defined(S32K_BUTTON_CAN_INSTANCE)
static flexcan_msgbuff_t g_halCanMasterRxMsg;
static flexcan_msgbuff_t g_halCanButtonRxMsg;

static EmbResult HalS32kCan_CopySdkToFrame(const flexcan_msgbuff_t *sdkMsg,
                                           HalCanFrame *frame)
{
    uint8_t i;

    if (sdkMsg == 0 || frame == 0)
        return EMB_EINVAL;

    (void)memset(frame, 0, sizeof(*frame));
    frame->id = sdkMsg->msgId;
    frame->dlc = sdkMsg->dataLen;
    frame->isExtendedId = (uint8_t)((sdkMsg->cs & CAN_CS_IDE_MASK) != 0U ? 1U : 0U);

    if (frame->dlc > 8U)
        frame->dlc = 8U;

    for (i = 0U; i < frame->dlc; i++)
        frame->data[i] = sdkMsg->data[i];

    return EMB_OK;
}

static void HalS32kCan_PrepareDataInfo(const HalCanFrame *frame,
                                       flexcan_data_info_t *dataInfo)
{
    (void)memset(dataInfo, 0, sizeof(*dataInfo));
    dataInfo->data_length = frame->dlc;
    dataInfo->msg_id_type = (frame->isExtendedId != 0U) ? FLEXCAN_MSG_ID_EXT : FLEXCAN_MSG_ID_STD;
    dataInfo->enable_brs = false;
    dataInfo->fd_enable = false;
    dataInfo->fd_padding = 0U;
    dataInfo->is_remote = false;
}

static EmbResult HalS32kCan_ConfigRx(HalS32kCanPort *port)
{
    status_t status;
    flexcan_data_info_t dataInfo;

    if (port == 0)
        return EMB_EINVAL;

    (void)memset(&dataInfo, 0, sizeof(dataInfo));
    dataInfo.data_length = 8U;
    dataInfo.msg_id_type = FLEXCAN_MSG_ID_STD;
    dataInfo.enable_brs = false;
    dataInfo.fd_enable = false;
    dataInfo.fd_padding = 0U;
    dataInfo.is_remote = false;

    status = FLEXCAN_DRV_ConfigRxMb((uint8_t)port->instance,
                                    (uint8_t)port->rxMb,
                                    &dataInfo,
                                    0U);
    if (status != STATUS_SUCCESS)
        return EMB_EIO;

    if (port->binding == HAL_S32K_CAN_BIND_MASTER)
    {
        status = FLEXCAN_DRV_Receive((uint8_t)port->instance,
                                     (uint8_t)port->rxMb,
                                     &g_halCanMasterRxMsg);
    }
    else if (port->binding == HAL_S32K_CAN_BIND_BUTTON)
    {
        status = FLEXCAN_DRV_Receive((uint8_t)port->instance,
                                     (uint8_t)port->rxMb,
                                     &g_halCanButtonRxMsg);
    }
    else
    {
        return EMB_EINVAL;
    }

    return (status == STATUS_SUCCESS) ? EMB_OK : EMB_EIO;
}

static flexcan_msgbuff_t *HalS32kCan_GetRxStorage(const HalS32kCanPort *port)
{
    if (port == 0)
        return 0;

    if (port->binding == HAL_S32K_CAN_BIND_MASTER)
        return &g_halCanMasterRxMsg;
    if (port->binding == HAL_S32K_CAN_BIND_BUTTON)
        return &g_halCanButtonRxMsg;

    return 0;
}

static EmbResult HalS32kCan_BindInitMaster(HalS32kCanPort *port)
{
#if defined(S32K_MASTER_CAN_INSTANCE) && defined(S32K_MASTER_CAN_STATE) && defined(S32K_MASTER_CAN_INIT_CONFIG)
    if (port == 0)
        return EMB_EINVAL;

    port->instance = S32K_MASTER_CAN_INSTANCE;
    if (FLEXCAN_DRV_Init((uint8_t)S32K_MASTER_CAN_INSTANCE,
                         &S32K_MASTER_CAN_STATE,
                         &S32K_MASTER_CAN_INIT_CONFIG) != STATUS_SUCCESS)
    {
        return EMB_EIO;
    }

    return EMB_OK;
#else
    (void)port;
    return EMB_EUNSUPPORTED;
#endif
}

static EmbResult HalS32kCan_BindInitButton(HalS32kCanPort *port)
{
#if defined(S32K_BUTTON_CAN_INSTANCE) && defined(S32K_BUTTON_CAN_STATE) && defined(S32K_BUTTON_CAN_INIT_CONFIG)
    if (port == 0)
        return EMB_EINVAL;

    port->instance = S32K_BUTTON_CAN_INSTANCE;
    if (FLEXCAN_DRV_Init((uint8_t)S32K_BUTTON_CAN_INSTANCE,
                         &S32K_BUTTON_CAN_STATE,
                         &S32K_BUTTON_CAN_INIT_CONFIG) != STATUS_SUCCESS)
    {
        return EMB_EIO;
    }

    return EMB_OK;
#else
    (void)port;
    return EMB_EUNSUPPORTED;
#endif
}
#endif

EmbResult HalS32kCan_Init(HalS32kCanPort *port)
{
    if (port == 0)
        return EMB_EINVAL;

#if defined(S32K_MASTER_CAN_INSTANCE) || defined(S32K_BUTTON_CAN_INSTANCE)
    if (port->binding == HAL_S32K_CAN_BIND_MASTER)
        return HalS32kCan_BindInitMaster(port);
    if (port->binding == HAL_S32K_CAN_BIND_BUTTON)
        return HalS32kCan_BindInitButton(port);
    return EMB_EINVAL;
#else
    (void)port;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kCan_Start(HalS32kCanPort *port)
{
#if defined(S32K_MASTER_CAN_INSTANCE) || defined(S32K_BUTTON_CAN_INSTANCE)
    return HalS32kCan_ConfigRx(port);
#else
    (void)port;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kCan_IsTxBusy(HalS32kCanPort *port, uint8_t *outBusy)
{
#if defined(S32K_MASTER_CAN_INSTANCE) || defined(S32K_BUTTON_CAN_INSTANCE)
    status_t status;

    if (port == 0 || outBusy == 0)
        return EMB_EINVAL;

    status = FLEXCAN_DRV_GetTransferStatus((uint8_t)port->instance,
                                           (uint8_t)port->txMb);
    if (status == STATUS_BUSY)
    {
        *outBusy = 1U;
        return EMB_OK;
    }

    *outBusy = 0U;
    return EMB_OK;
#else
    (void)port;
    (void)outBusy;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kCan_Tx(HalS32kCanPort *port, const HalCanFrame *frame)
{
#if defined(S32K_MASTER_CAN_INSTANCE) || defined(S32K_BUTTON_CAN_INSTANCE)
    flexcan_data_info_t dataInfo;

    if (port == 0 || frame == 0)
        return EMB_EINVAL;
    if (frame->dlc > 8U)
        return EMB_EINVAL;

    HalS32kCan_PrepareDataInfo(frame, &dataInfo);

    if (FLEXCAN_DRV_ConfigTxMb((uint8_t)port->instance,
                               (uint8_t)port->txMb,
                               &dataInfo,
                               frame->id) != STATUS_SUCCESS)
    {
        return EMB_EIO;
    }

    if (FLEXCAN_DRV_Send((uint8_t)port->instance,
                         (uint8_t)port->txMb,
                         &dataInfo,
                         frame->id,
                         frame->data) != STATUS_SUCCESS)
    {
        return EMB_EIO;
    }

    return EMB_OK;
#else
    (void)port;
    (void)frame;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kCan_TryRead(HalS32kCanPort *port, HalCanFrame *frame)
{
#if defined(S32K_MASTER_CAN_INSTANCE) || defined(S32K_BUTTON_CAN_INSTANCE)
    flexcan_msgbuff_t *sdkMsg;
    status_t status;

    if (port == 0 || frame == 0)
        return EMB_EINVAL;

    status = FLEXCAN_DRV_GetTransferStatus((uint8_t)port->instance,
                                           (uint8_t)port->rxMb);
    if (status == STATUS_BUSY)
        return EMB_EBUSY;
    if (status != STATUS_SUCCESS)
        return EMB_EIO;

    sdkMsg = HalS32kCan_GetRxStorage(port);
    if (sdkMsg == 0)
        return EMB_EIO;

    if (HalS32kCan_CopySdkToFrame(sdkMsg, frame) != EMB_OK)
        return EMB_EIO;

    if (FLEXCAN_DRV_Receive((uint8_t)port->instance,
                            (uint8_t)port->rxMb,
                            sdkMsg) != STATUS_SUCCESS)
    {
        return EMB_EIO;
    }

    return EMB_OK;
#else
    (void)port;
    (void)frame;
    return EMB_EUNSUPPORTED;
#endif
}
