#include "platform/s32k_bindings.h"
#include "hal/hal_s32k_uart.h"

#if defined(S32K_MASTER_UART_INSTANCE) || defined(S32K_BUTTON_UART_INSTANCE)
static uint8_t g_halUartMasterRxByte;
static uint8_t g_halUartButtonRxByte;
static uint8_t g_halUartMasterRxArmed;
static uint8_t g_halUartButtonRxArmed;

static uint8_t *HalS32kUart_GetRxStorage(const HalS32kUartPort *port)
{
    if (port == 0)
        return 0;
    if (port->binding == HAL_S32K_UART_BIND_MASTER)
        return &g_halUartMasterRxByte;
    if (port->binding == HAL_S32K_UART_BIND_BUTTON)
        return &g_halUartButtonRxByte;
    return 0;
}

static uint8_t *HalS32kUart_GetRxArmedFlag(const HalS32kUartPort *port)
{
    if (port == 0)
        return 0;
    if (port->binding == HAL_S32K_UART_BIND_MASTER)
        return &g_halUartMasterRxArmed;
    if (port->binding == HAL_S32K_UART_BIND_BUTTON)
        return &g_halUartButtonRxArmed;
    return 0;
}

static EmbResult HalS32kUart_BindInitMaster(HalS32kUartPort *port)
{
#if defined(S32K_MASTER_UART_INSTANCE) && defined(S32K_MASTER_UART_STATE) && defined(S32K_MASTER_UART_INIT_CONFIG)
    if (port == 0)
        return EMB_EINVAL;

    port->instance = S32K_MASTER_UART_INSTANCE;
    if (LPUART_DRV_Init((uint32_t)S32K_MASTER_UART_INSTANCE,
                        &S32K_MASTER_UART_STATE,
                        &S32K_MASTER_UART_INIT_CONFIG) != STATUS_SUCCESS)
    {
        return EMB_EIO;
    }

    return EMB_OK;
#else
    (void)port;
    return EMB_EUNSUPPORTED;
#endif
}

static EmbResult HalS32kUart_BindInitButton(HalS32kUartPort *port)
{
#if defined(S32K_BUTTON_UART_INSTANCE) && defined(S32K_BUTTON_UART_STATE) && defined(S32K_BUTTON_UART_INIT_CONFIG)
    if (port == 0)
        return EMB_EINVAL;

    port->instance = S32K_BUTTON_UART_INSTANCE;
    if (LPUART_DRV_Init((uint32_t)S32K_BUTTON_UART_INSTANCE,
                        &S32K_BUTTON_UART_STATE,
                        &S32K_BUTTON_UART_INIT_CONFIG) != STATUS_SUCCESS)
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

EmbResult HalS32kUart_Init(HalS32kUartPort *port)
{
    if (port == 0)
        return EMB_EINVAL;

#if defined(S32K_MASTER_UART_INSTANCE) || defined(S32K_BUTTON_UART_INSTANCE)
    if (port->binding == HAL_S32K_UART_BIND_MASTER)
        return HalS32kUart_BindInitMaster(port);
    if (port->binding == HAL_S32K_UART_BIND_BUTTON)
        return HalS32kUart_BindInitButton(port);
    return EMB_EINVAL;
#else
    (void)port;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kUart_StartRx(HalS32kUartPort *port, uint8_t *buffer, uint32_t size)
{
#if defined(S32K_MASTER_UART_INSTANCE) || defined(S32K_BUTTON_UART_INSTANCE)
    uint8_t *armed;
    if (port == 0 || buffer == 0 || size == 0U)
        return EMB_EINVAL;

    if (LPUART_DRV_ReceiveData((uint32_t)port->instance, buffer, size) != STATUS_SUCCESS)
        return EMB_EIO;

    armed = HalS32kUart_GetRxArmedFlag(port);
    if (armed != 0)
        *armed = 1U;
    return EMB_OK;
#else
    (void)port;
    (void)buffer;
    (void)size;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kUart_PollRxByte(HalS32kUartPort *port, uint8_t *outByte)
{
#if defined(S32K_MASTER_UART_INSTANCE) || defined(S32K_BUTTON_UART_INSTANCE)
    uint8_t *storage;
    uint8_t *armed;
    uint32_t remaining = 0U;
    status_t status;

    if (port == 0 || outByte == 0)
        return EMB_EINVAL;

    storage = HalS32kUart_GetRxStorage(port);
    armed = HalS32kUart_GetRxArmedFlag(port);
    if (storage == 0 || armed == 0)
        return EMB_EINVAL;

    if (*armed == 0U)
    {
        if (HalS32kUart_StartRx(port, storage, 1U) != EMB_OK)
            return EMB_EIO;
        return EMB_EBUSY;
    }

    status = LPUART_DRV_GetReceiveStatus((uint32_t)port->instance, &remaining);
    if (status == STATUS_BUSY)
        return EMB_EBUSY;
    if (status != STATUS_SUCCESS)
    {
        *armed = 0U;
        return EMB_EIO;
    }

    *outByte = *storage;
    *armed = 0U;
    if (HalS32kUart_StartRx(port, storage, 1U) != EMB_OK)
        return EMB_EIO;

    return EMB_OK;
#else
    (void)port;
    (void)outByte;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kUart_IsTxBusy(HalS32kUartPort *port, uint8_t *outBusy)
{
#if defined(S32K_MASTER_UART_INSTANCE) || defined(S32K_BUTTON_UART_INSTANCE)
    uint32_t remaining = 0U;
    status_t status;

    if (port == 0 || outBusy == 0)
        return EMB_EINVAL;

    status = LPUART_DRV_GetTransmitStatus((uint32_t)port->instance, &remaining);
    if (status == STATUS_BUSY)
    {
        *outBusy = 1U;
        return EMB_OK;
    }
    if (status != STATUS_SUCCESS)
        return EMB_EIO;

    *outBusy = 0U;
    return EMB_OK;
#else
    (void)port;
    (void)outBusy;
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kUart_Tx(HalS32kUartPort *port, const uint8_t *data, uint32_t size)
{
#if defined(S32K_MASTER_UART_INSTANCE) || defined(S32K_BUTTON_UART_INSTANCE)
    if (port == 0 || data == 0 || size == 0U)
        return EMB_EINVAL;

    if (LPUART_DRV_SendData((uint32_t)port->instance, data, size) != STATUS_SUCCESS)
        return EMB_EIO;

    return EMB_OK;
#else
    (void)port;
    (void)data;
    (void)size;
    return EMB_EUNSUPPORTED;
#endif
}
