/*
 * master 노드용 실제 보드 바인딩 구현부다.
 * coordinator가 쓰는 LIN master 초기화와 callback bridge만 남겨,
 * SDK 세부사항은 IsoSdk 계층으로 격리한다.
 */
#include "runtime_io.h"

#include <stddef.h>
#include <string.h>

#include "../../isosdk/isosdk_board.h"
#include "../../isosdk/isosdk_lin.h"
#include "app/app_config.h"

#define RUNTIME_IO_MASTER_LIN_ADC_PID         0x24U
#define RUNTIME_IO_MASTER_LIN_OK_PID          0x25U
#define RUNTIME_IO_MASTER_LIN_OK_TOKEN        0xA5U
#define RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS   100U

static LinModule       *g_runtime_io_lin_module;
static IsoSdkLinContext g_runtime_io_lin_context;

static LinEventId RuntimeIo_LinEventFromIsoSdk(uint8_t event_id)
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

static void RuntimeIo_LinIsoSdkEvent(void *context, uint8_t event_id, uint8_t current_pid)
{
    LinEventId lin_event_id;

    (void)context;
    lin_event_id = RuntimeIo_LinEventFromIsoSdk(event_id);
    if (lin_event_id != LIN_EVENT_NONE)
    {
        RuntimeIo_LinNotifyEvent(lin_event_id, current_pid);
    }
}

static InfraStatus RuntimeIo_LinInitSdk(void *context)
{
    IsoSdkLinContext *lin_context;

    lin_context = (IsoSdkLinContext *)context;
    if (lin_context == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (IsoSdk_LinIsSupported() == 0U)
    {
        return INFRA_STATUS_UNSUPPORTED;
    }

    return (IsoSdk_LinInit(lin_context,
                           ISOSDK_LIN_ROLE_MASTER,
                           lin_context->timeout_ticks,
                           RuntimeIo_LinIsoSdkEvent,
                           NULL) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

static InfraStatus RuntimeIo_LinMasterSendHeaderSdk(void *context, uint8_t pid)
{
    IsoSdkLinContext *lin_context;

    lin_context = (IsoSdkLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U))
    {
        return INFRA_STATUS_NOT_READY;
    }

    return (IsoSdk_LinMasterSendHeader(lin_context, pid) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

static InfraStatus RuntimeIo_LinStartReceiveSdk(void *context, uint8_t *buffer, uint8_t length)
{
    IsoSdkLinContext *lin_context;

    lin_context = (IsoSdkLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U) ||
        (buffer == NULL) || (length == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    return (IsoSdk_LinStartReceive(lin_context, buffer, length) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

static InfraStatus RuntimeIo_LinStartSendSdk(void *context, const uint8_t *buffer, uint8_t length)
{
    IsoSdkLinContext *lin_context;

    lin_context = (IsoSdkLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U) ||
        (buffer == NULL) || (length == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    return (IsoSdk_LinStartSend(lin_context, buffer, length) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

static void RuntimeIo_LinGotoIdleSdk(void *context)
{
    IsoSdkLinContext *lin_context;

    lin_context = (IsoSdkLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U))
    {
        return;
    }

    IsoSdk_LinGotoIdle(lin_context);
}

static void RuntimeIo_LinSetTimeoutSdk(void *context, uint16_t timeout_ticks)
{
    IsoSdkLinContext *lin_context;

    lin_context = (IsoSdkLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U))
    {
        return;
    }

    IsoSdk_LinSetTimeout(lin_context, timeout_ticks);
}

static void RuntimeIo_LinServiceTickSdk(void *context)
{
    IsoSdkLinContext *lin_context;

    lin_context = (IsoSdkLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U))
    {
        return;
    }

    IsoSdk_LinServiceTick(lin_context);
}

InfraStatus RuntimeIo_BoardInit(void)
{
    if (IsoSdk_BoardInit() == 0U)
    {
        return INFRA_STATUS_IO_ERROR;
    }

    IsoSdk_BoardEnableLinTransceiver();
    return INFRA_STATUS_OK;
}

uint8_t RuntimeIo_GetLocalNodeId(void)
{
    return APP_NODE_ID_MASTER;
}

void RuntimeIo_AttachLinModule(LinModule *module)
{
    g_runtime_io_lin_module = module;
}

void RuntimeIo_LinNotifyEvent(LinEventId event_id, uint8_t current_pid)
{
    if (g_runtime_io_lin_module == NULL)
    {
        return;
    }

    LinModule_OnEvent(g_runtime_io_lin_module, event_id, current_pid);
}

InfraStatus RuntimeIo_GetMasterLinConfig(LinConfig *out_config)
{
    if (out_config == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(out_config, 0, sizeof(*out_config));
    (void)memset(&g_runtime_io_lin_context, 0, sizeof(g_runtime_io_lin_context));
    out_config->role = LIN_ROLE_MASTER;
    out_config->pid_status = RUNTIME_IO_MASTER_LIN_ADC_PID;
    out_config->pid_ok = RUNTIME_IO_MASTER_LIN_OK_PID;
    out_config->ok_token = RUNTIME_IO_MASTER_LIN_OK_TOKEN;
    out_config->status_frame_size = 8U;
    out_config->ok_frame_size = 1U;
    out_config->timeout_ticks = RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS;
    out_config->poll_period_ms = APP_TASK_LIN_POLL_MS;
    g_runtime_io_lin_context.timeout_ticks = RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS;
    out_config->binding.init_fn = RuntimeIo_LinInitSdk;
    out_config->binding.master_send_header_fn = RuntimeIo_LinMasterSendHeaderSdk;
    out_config->binding.start_receive_fn = RuntimeIo_LinStartReceiveSdk;
    out_config->binding.start_send_fn = RuntimeIo_LinStartSendSdk;
    out_config->binding.goto_idle_fn = RuntimeIo_LinGotoIdleSdk;
    out_config->binding.set_timeout_fn = RuntimeIo_LinSetTimeoutSdk;
    out_config->binding.service_tick_fn = RuntimeIo_LinServiceTickSdk;
    out_config->binding.context = &g_runtime_io_lin_context;
    return (IsoSdk_LinIsSupported() != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_UNSUPPORTED;
}
