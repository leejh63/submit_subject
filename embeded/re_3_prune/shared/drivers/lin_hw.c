#include "lin_hw.h"

#include <stddef.h>
#include <string.h>

#include "../../platform/s32k_sdk/isosdk_lin.h"

typedef struct
{
    LinModule        *module;
    IsoSdkLinContext  sdk_context;
    uint8_t           role;
    uint16_t          timeout_ticks;
} LinHwState;

static LinHwState s_lin_hw;

static LinEventId LinHw_EventFromIsoSdk(uint8_t event_id)
{
    switch (event_id)
    {
        case ISOSDK_LIN_EVENT_PID_OK:
            return LIN_EVENT_PID_OK;

        case ISOSDK_LIN_EVENT_RX_DONE:
            return LIN_EVENT_RX_DONE;

        case ISOSDK_LIN_EVENT_TX_DONE:
            return LIN_EVENT_TX_DONE;

        case ISOSDK_LIN_EVENT_ERROR:
            return LIN_EVENT_ERROR;

        default:
            return LIN_EVENT_NONE;
    }
}

static void LinHw_OnIsoSdkEvent(void *context, uint8_t event_id, uint8_t current_pid)
{
    LinEventId lin_event_id;

    (void)context;
    lin_event_id = LinHw_EventFromIsoSdk(event_id);
    if ((s_lin_hw.module != NULL) && (lin_event_id != LIN_EVENT_NONE))
    {
        LinModule_OnEvent(s_lin_hw.module, lin_event_id, current_pid);
    }
}

uint8_t LinHw_IsSupported(void)
{
    return IsoSdk_LinIsSupported();
}

void LinHw_Configure(uint8_t role, uint16_t timeout_ticks)
{
    (void)memset(&s_lin_hw.sdk_context, 0, sizeof(s_lin_hw.sdk_context));
    s_lin_hw.role = role;
    s_lin_hw.timeout_ticks = timeout_ticks;
    s_lin_hw.sdk_context.timeout_ticks = timeout_ticks;
}

void LinHw_AttachModule(LinModule *module)
{
    s_lin_hw.module = module;
}

InfraStatus LinHw_Init(void *context)
{
    uint8_t iso_role;

    (void)context;
    if (IsoSdk_LinIsSupported() == 0U)
    {
        return INFRA_STATUS_UNSUPPORTED;
    }

    iso_role = (s_lin_hw.role == LIN_ROLE_MASTER) ? ISOSDK_LIN_ROLE_MASTER : ISOSDK_LIN_ROLE_SLAVE;
    return (IsoSdk_LinInit(&s_lin_hw.sdk_context,
                           iso_role,
                           s_lin_hw.timeout_ticks,
                           LinHw_OnIsoSdkEvent,
                           NULL) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

InfraStatus LinHw_MasterSendHeader(void *context, uint8_t pid)
{
    (void)context;
    return (IsoSdk_LinMasterSendHeader(&s_lin_hw.sdk_context, pid) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

InfraStatus LinHw_StartReceive(void *context, uint8_t *buffer, uint8_t length)
{
    (void)context;
    return (IsoSdk_LinStartReceive(&s_lin_hw.sdk_context, buffer, length) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

InfraStatus LinHw_StartSend(void *context, const uint8_t *buffer, uint8_t length)
{
    (void)context;
    return (IsoSdk_LinStartSend(&s_lin_hw.sdk_context, buffer, length) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

void LinHw_GotoIdle(void *context)
{
    (void)context;
    IsoSdk_LinGotoIdle(&s_lin_hw.sdk_context);
}

void LinHw_SetTimeout(void *context, uint16_t timeout_ticks)
{
    (void)context;
    s_lin_hw.timeout_ticks = timeout_ticks;
    IsoSdk_LinSetTimeout(&s_lin_hw.sdk_context, timeout_ticks);
}

void LinHw_ServiceTick(void *context)
{
    (void)context;
    IsoSdk_LinServiceTick(&s_lin_hw.sdk_context);
}
