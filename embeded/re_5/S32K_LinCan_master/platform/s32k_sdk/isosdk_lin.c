#include "isosdk_lin.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "isosdk_sdk_bindings.h"

#ifdef ISOSDK_SDK_HAS_LIN

static IsoSdkLinContext *s_iso_sdk_lin_context;

static void IsoSdk_LinDispatchEvent(uint8_t event_id, uint8_t current_pid)
{
    if ((s_iso_sdk_lin_context == NULL) ||
        (s_iso_sdk_lin_context->event_cb == NULL))
    {
        return;
    }

    s_iso_sdk_lin_context->event_cb(s_iso_sdk_lin_context->event_context,
                                    event_id,
                                    current_pid);
}

static void IsoSdk_LinSdkCallback(uint32_t instance, void *lin_state_ptr)
{
    lin_state_t *lin_state;
    uint8_t      event_id;
    uint8_t      current_pid;

    (void)instance;

    lin_state = (lin_state_t *)lin_state_ptr;
    if (lin_state == NULL)
    {
        return;
    }

    current_pid = lin_state->currentId;
    if (lin_state->timeoutCounterFlag != false)
    {
        lin_state->timeoutCounterFlag = false;
        IsoSdk_LinDispatchEvent(ISOSDK_LIN_EVENT_ERROR, current_pid);
        return;
    }

    if (lin_state->currentEventId == LIN_RECV_BREAK_FIELD_OK)
    {
        if ((s_iso_sdk_lin_context != NULL) && (s_iso_sdk_lin_context->initialized != 0U))
        {
            LIN_DRV_SetTimeoutCounter(ISOSDK_SDK_LIN_INSTANCE,
                                      s_iso_sdk_lin_context->timeout_ticks);
        }
        return;
    }

    event_id = ISOSDK_LIN_EVENT_NONE;
    switch (lin_state->currentEventId)
    {
        case LIN_PID_OK:
            event_id = ISOSDK_LIN_EVENT_PID_OK;
            break;

        case LIN_RX_COMPLETED:
            event_id = ISOSDK_LIN_EVENT_RX_DONE;
            break;

        case LIN_TX_COMPLETED:
            event_id = ISOSDK_LIN_EVENT_TX_DONE;
            break;

        case LIN_PID_ERROR:
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_RX_OVERRUN:
        case LIN_SYNC_ERROR:
            event_id = ISOSDK_LIN_EVENT_ERROR;
            break;

        default:
            break;
    }

    if (event_id != ISOSDK_LIN_EVENT_NONE)
    {
        IsoSdk_LinDispatchEvent(event_id, current_pid);
    }
}

uint8_t IsoSdk_LinIsSupported(void)
{
    return 1U;
}

uint8_t IsoSdk_LinInit(IsoSdkLinContext *context,
                       uint8_t role,
                       uint16_t timeout_ticks,
                       IsoSdkLinEventCallback event_cb,
                       void *event_context)
{
    status_t status;

    if (context == NULL)
    {
        return 0U;
    }

    (void)memset(context, 0, sizeof(*context));
    context->timeout_ticks = timeout_ticks;
    context->role = role;
    context->event_cb = event_cb;
    context->event_context = event_context;

    ISOSDK_SDK_LIN_INIT_CONFIG.nodeFunction =
        (role == ISOSDK_LIN_ROLE_MASTER) ? MASTER : SLAVE;
    ISOSDK_SDK_LIN_INIT_CONFIG.autobaudEnable = false;

    status = LIN_DRV_Init(ISOSDK_SDK_LIN_INSTANCE,
                          &ISOSDK_SDK_LIN_INIT_CONFIG,
                          &ISOSDK_SDK_LIN_STATE);
    if (status != STATUS_SUCCESS)
    {
        return 0U;
    }

    s_iso_sdk_lin_context = context;
    (void)LIN_DRV_InstallCallback(ISOSDK_SDK_LIN_INSTANCE, IsoSdk_LinSdkCallback);
    context->initialized = 1U;
    return 1U;
}

uint8_t IsoSdk_LinMasterSendHeader(IsoSdkLinContext *context, uint8_t pid)
{
    if ((context == NULL) || (context->initialized == 0U))
    {
        return 0U;
    }

    return (LIN_DRV_MasterSendHeader(ISOSDK_SDK_LIN_INSTANCE, pid) == STATUS_SUCCESS) ? 1U : 0U;
}

uint8_t IsoSdk_LinStartReceive(IsoSdkLinContext *context, uint8_t *buffer, uint8_t length)
{
    if ((context == NULL) || (context->initialized == 0U) ||
        (buffer == NULL) || (length == 0U))
    {
        return 0U;
    }

    return (LIN_DRV_ReceiveFrameData(ISOSDK_SDK_LIN_INSTANCE, buffer, length) == STATUS_SUCCESS) ? 1U : 0U;
}

uint8_t IsoSdk_LinStartSend(IsoSdkLinContext *context, const uint8_t *buffer, uint8_t length)
{
    if ((context == NULL) || (context->initialized == 0U) ||
        (buffer == NULL) || (length == 0U))
    {
        return 0U;
    }

    return (LIN_DRV_SendFrameData(ISOSDK_SDK_LIN_INSTANCE, buffer, length) == STATUS_SUCCESS) ? 1U : 0U;
}

void IsoSdk_LinGotoIdle(IsoSdkLinContext *context)
{
    if ((context == NULL) || (context->initialized == 0U))
    {
        return;
    }

    (void)LIN_DRV_GotoIdleState(ISOSDK_SDK_LIN_INSTANCE);
}

void IsoSdk_LinSetTimeout(IsoSdkLinContext *context, uint16_t timeout_ticks)
{
    if ((context == NULL) || (context->initialized == 0U))
    {
        return;
    }

    context->timeout_ticks = timeout_ticks;
    LIN_DRV_SetTimeoutCounter(ISOSDK_SDK_LIN_INSTANCE, timeout_ticks);
}

void IsoSdk_LinServiceTick(IsoSdkLinContext *context)
{
    if ((context == NULL) || (context->initialized == 0U))
    {
        return;
    }

    LIN_DRV_TimeoutService(ISOSDK_SDK_LIN_INSTANCE);
}

#else

uint8_t IsoSdk_LinIsSupported(void)
{
    return 0U;
}

uint8_t IsoSdk_LinInit(IsoSdkLinContext *context,
                       uint8_t role,
                       uint16_t timeout_ticks,
                       IsoSdkLinEventCallback event_cb,
                       void *event_context)
{
    (void)context;
    (void)role;
    (void)timeout_ticks;
    (void)event_cb;
    (void)event_context;
    return 0U;
}

uint8_t IsoSdk_LinMasterSendHeader(IsoSdkLinContext *context, uint8_t pid)
{
    (void)context;
    (void)pid;
    return 0U;
}

uint8_t IsoSdk_LinStartReceive(IsoSdkLinContext *context, uint8_t *buffer, uint8_t length)
{
    (void)context;
    (void)buffer;
    (void)length;
    return 0U;
}

uint8_t IsoSdk_LinStartSend(IsoSdkLinContext *context, const uint8_t *buffer, uint8_t length)
{
    (void)context;
    (void)buffer;
    (void)length;
    return 0U;
}

void IsoSdk_LinGotoIdle(IsoSdkLinContext *context)
{
    (void)context;
}

void IsoSdk_LinSetTimeout(IsoSdkLinContext *context, uint16_t timeout_ticks)
{
    (void)context;
    (void)timeout_ticks;
}

void IsoSdk_LinServiceTick(IsoSdkLinContext *context)
{
    (void)context;
}

#endif
