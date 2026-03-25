#include "hal/hal_s32k_lin.h"
#include "platform/s32k_bindings.h"

static EmbResult HalS32kLin_InitSensorSlave(HalS32kLinPort *port)
{
    port->instance = S32K_LIN_SENSOR_LIN_INSTANCE;
    S32K_LIN_SENSOR_LIN_INIT_CONFIG.autobaudEnable = false;
    if (LIN_DRV_Init(port->instance,
                     &S32K_LIN_SENSOR_LIN_INIT_CONFIG,
                     &S32K_LIN_SENSOR_LIN_STATE) != STATUS_SUCCESS)
    {
        return EMB_EIO;
    }

    return EMB_OK;
}

static EmbResult HalS32kLin_InitMaster(HalS32kLinPort *port)
{
#if defined(S32K_MASTER_LIN_INSTANCE) && defined(S32K_MASTER_LIN_INIT_CONFIG) && defined(S32K_MASTER_LIN_STATE)
    port->instance = S32K_MASTER_LIN_INSTANCE;
    if (LIN_DRV_Init(port->instance,
                     &S32K_MASTER_LIN_INIT_CONFIG,
                     &S32K_MASTER_LIN_STATE) != STATUS_SUCCESS)
    {
        return EMB_EIO;
    }
    return EMB_OK;
#else
    (void)port;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kLin_Init(HalS32kLinPort *port)
{
    if (port == 0)
        return EMB_EINVAL;

    switch (port->binding)
    {
        case HAL_S32K_LIN_BIND_SENSOR_SLAVE:
            return HalS32kLin_InitSensorSlave(port);

        case HAL_S32K_LIN_BIND_MASTER:
            return HalS32kLin_InitMaster(port);

        case HAL_S32K_LIN_BIND_CUSTOM:
        default:
            return EMB_EUNSUPPORTED;
    }
}

EmbResult HalS32kLin_InstallCallback(HalS32kLinPort *port, HalS32kLinCallback callback)
{
    if (port == 0 || callback == 0)
        return EMB_EINVAL;

    if (LIN_DRV_InstallCallback(port->instance, callback) != STATUS_SUCCESS)
        return EMB_EIO;

    return EMB_OK;
}

EmbResult HalS32kLin_Send(HalS32kLinPort *port, uint8_t *data, uint8_t size)
{
    if (port == 0 || data == 0 || size == 0U)
        return EMB_EINVAL;

    if (LIN_DRV_SendFrameData(port->instance, data, size) != STATUS_SUCCESS)
        return EMB_EIO;

    return EMB_OK;
}

EmbResult HalS32kLin_Receive(HalS32kLinPort *port, uint8_t *data, uint8_t size)
{
    if (port == 0 || data == 0 || size == 0U)
        return EMB_EINVAL;

    if (LIN_DRV_ReceiveFrameData(port->instance, data, size) != STATUS_SUCCESS)
        return EMB_EIO;

    return EMB_OK;
}

EmbResult HalS32kLin_GoIdle(HalS32kLinPort *port)
{
    if (port == 0)
        return EMB_EINVAL;

    if (LIN_DRV_GotoIdleState(port->instance) != STATUS_SUCCESS)
        return EMB_EIO;

    return EMB_OK;
}

EmbResult HalS32kLin_SetTimeout(HalS32kLinPort *port, uint16_t timeout)
{
    if (port == 0)
        return EMB_EINVAL;

    LIN_DRV_SetTimeoutCounter(port->instance, timeout);
    port->timeoutTicks = timeout;
    return EMB_OK;
}

EmbResult HalS32kLin_MasterSendHeader(HalS32kLinPort *port, uint8_t pid)
{
    if (port == 0)
        return EMB_EINVAL;

    if (LIN_DRV_MasterSendHeader(port->instance, pid) != STATUS_SUCCESS)
        return EMB_EIO;

    return EMB_OK;
}
