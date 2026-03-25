/*
 * LIN sensor slave용 프로젝트 바인딩 구현부다.
 * 역할별 상수와 주기 설정을 조립하고,
 * 실제 하드웨어 접근은 shared driver 계층에 위임한다.
 */
#include "runtime_io.h"

#include <stddef.h>
#include <string.h>

#include "../drivers/adc_hw.h"
#include "../drivers/board_hw.h"
#include "../drivers/lin_hw.h"
#include "../app/app_config.h"

#define RUNTIME_IO_MASTER_LIN_ADC_PID         0x24U
#define RUNTIME_IO_MASTER_LIN_OK_PID          0x25U
#define RUNTIME_IO_MASTER_LIN_OK_TOKEN        0xA5U
#define RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS   100U

#define RUNTIME_IO_ADC_RANGE_MAX              4096U
#define RUNTIME_IO_ADC_SAFE_MAX               (RUNTIME_IO_ADC_RANGE_MAX / 3U)
#define RUNTIME_IO_ADC_WARNING_MAX            ((RUNTIME_IO_ADC_RANGE_MAX * 2U) / 3U)
#define RUNTIME_IO_ADC_EMERGENCY_MIN          ((RUNTIME_IO_ADC_RANGE_MAX * 5U) / 6U)

static LinModule *g_runtime_io_lin_module;

InfraStatus RuntimeIo_BoardInit(void)
{
    InfraStatus status;

    status = BoardHw_Init();
    if (status != INFRA_STATUS_OK)
    {
        return status;
    }

    BoardHw_EnableLinTransceiver();
    return INFRA_STATUS_OK;
}

uint8_t RuntimeIo_GetLocalNodeId(void)
{
    return APP_NODE_ID_SLAVE2;
}

InfraStatus RuntimeIo_GetSlave2LedConfig(LedConfig *out_config)
{
    return BoardHw_GetRgbLedConfig(out_config);
}

InfraStatus RuntimeIo_GetSlave2AdcConfig(AdcConfig *out_config)
{
    if (out_config == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(out_config, 0, sizeof(*out_config));
    out_config->init_fn = AdcHw_Init;
    out_config->sample_fn = AdcHw_Sample;
    out_config->hw_context = NULL;
    out_config->sample_period_ms = APP_TASK_ADC_MS;
    out_config->range_max = RUNTIME_IO_ADC_RANGE_MAX;
    out_config->safe_max = RUNTIME_IO_ADC_SAFE_MAX;
    out_config->warning_max = RUNTIME_IO_ADC_WARNING_MAX;
    out_config->emergency_min = RUNTIME_IO_ADC_EMERGENCY_MIN;
    out_config->blocking_mode = 1U;
    return (AdcHw_IsSupported() != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_UNSUPPORTED;
}

void RuntimeIo_AttachLinModule(LinModule *module)
{
    g_runtime_io_lin_module = module;
    LinHw_AttachModule(module);
}

void RuntimeIo_LinNotifyEvent(LinEventId event_id, uint8_t current_pid)
{
    if (g_runtime_io_lin_module == NULL)
    {
        return;
    }

    LinModule_OnEvent(g_runtime_io_lin_module, event_id, current_pid);
}

InfraStatus RuntimeIo_GetSlaveLinConfig(LinConfig *out_config)
{
    if (out_config == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(out_config, 0, sizeof(*out_config));
    out_config->role = LIN_ROLE_SLAVE;
    out_config->pid_status = RUNTIME_IO_MASTER_LIN_ADC_PID;
    out_config->pid_ok = RUNTIME_IO_MASTER_LIN_OK_PID;
    out_config->ok_token = RUNTIME_IO_MASTER_LIN_OK_TOKEN;
    out_config->status_frame_size = 8U;
    out_config->ok_frame_size = 1U;
    out_config->timeout_ticks = RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS;
    out_config->poll_period_ms = APP_TASK_LIN_POLL_MS;
    out_config->binding.init_fn = LinHw_Init;
    out_config->binding.master_send_header_fn = LinHw_MasterSendHeader;
    out_config->binding.start_receive_fn = LinHw_StartReceive;
    out_config->binding.start_send_fn = LinHw_StartSend;
    out_config->binding.goto_idle_fn = LinHw_GotoIdle;
    out_config->binding.set_timeout_fn = LinHw_SetTimeout;
    out_config->binding.service_tick_fn = LinHw_ServiceTick;
    out_config->binding.context = NULL;

    LinHw_Configure(LIN_ROLE_SLAVE, RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS);
    return (LinHw_IsSupported() != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_UNSUPPORTED;
}
