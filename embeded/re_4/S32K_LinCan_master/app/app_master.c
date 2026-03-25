/*
 * master 노드 policy 구현부다.
 * 시스템이 언제 emergency인지 판단하고,
 * slave1 통지 시점과 slave2 latch 해제 가능 시점을 결정한다.
 */
#include "app_master.h"

#include <stddef.h>
#include <stdio.h>

#include "app_config.h"
#include "app_core_internal.h"
#include "../runtime/runtime_io.h"

/*
 * master 역할의 통신 모듈을 초기화한다.
 * coordinator는 항상 console, CAN, LIN이 필요하므로,
 * startup은 이 공통 기능을 켜는 데 집중한다.
 */
InfraStatus AppMaster_Init(AppCore *app)
{
    LinConfig lin_config;

    if (app == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (AppCore_InitConsoleCan(app) != INFRA_STATUS_OK)
    {
        return INFRA_STATUS_IO_ERROR;
    }

    if ((RuntimeIo_GetMasterLinConfig(&lin_config) == INFRA_STATUS_OK) &&
        (LinModule_Init(&app->lin_module, &lin_config) == INFRA_STATUS_OK))
    {
        RuntimeIo_AttachLinModule(&app->lin_module);
        app->lin_enabled = 1U;
    }
    else
    {
        AppCore_SetLinLinkText(app, "binding req");
    }

    return INFRA_STATUS_OK;
}

/*
 * slave1에서 온 로컬/원격 승인 요청을 처리한다.
 * master는 센서 상태가 active emergency 구간을 벗어난 경우에만,
 * 요청을 slave2로 전달한다.
 */
void AppMaster_RequestOk(AppCore *app)
{
    LinStatusFrame status;

    if ((app == NULL) || (app->lin_enabled == 0U))
    {
        AppCore_SetResultText(app, "[wait] lin not ready");
        return;
    }

    if (LinModule_GetLatestStatus(&app->lin_module, &status) == 0U)
    {
        AppCore_SetButtonText(app, "waiting lin");
        AppCore_SetResultText(app, "[wait] no lin status");
        return;
    }

    if (status.zone == LIN_ZONE_EMERGENCY)
    {
        AppCore_SetButtonText(app, "denied");
        AppCore_SetResultText(app, "[deny] adc still emergency");
        return;
    }

    if (status.emergency_latched == 0U)
    {
        AppCore_SetButtonText(app, "already clear");
        AppCore_SetResultText(app, "[info] slave not locked");
        return;
    }

    app->master_slave1_ok_pending = 1U;
    (void)LinModule_RequestOk(&app->lin_module);
    AppCore_SetButtonText(app, "ok queued");
    AppCore_SetLinLinkText(app, "ok pending");
    AppCore_SetResultText(app, "[queued] slave1 ok -> lin ok");
}

/*
 * 새로 수신한 slave2 상태 프레임을 해석한다.
 * UI 상태를 갱신하고 emergency 전이를 추적하며,
 * slave1으로 보낼 CAN 동작을 결정하는 master policy의 핵심이다.
 */
void AppMaster_HandleFreshLinStatus(AppCore *app, const LinStatusFrame *status)
{
    uint8_t emergency_active;
    char    buffer[APP_CONSOLE_RESULT_VIEW_SIZE];

    if ((app == NULL) || (status == NULL))
    {
        return;
    }

    (void)snprintf(buffer,
                   sizeof(buffer),
                   "%u (%s, lock=%u)",
                   (unsigned int)status->adc_value,
                   AppCore_GetLinZoneText(status->zone),
                   (unsigned int)status->emergency_latched);
    AppCore_SetAdcText(app, buffer);

    (void)snprintf(buffer,
                   sizeof(buffer),
                   "adc in : %u (%s)",
                   (unsigned int)status->adc_value,
                   AppCore_GetLinZoneText(status->zone));
    AppCore_SetLinInputText(app, buffer);
    AppCore_SetLinLinkText(app, "ok");

    emergency_active = ((status->zone == LIN_ZONE_EMERGENCY) ||
                        (status->emergency_latched != 0U)) ? 1U : 0U;
    AppCore_SetModeText(app, (emergency_active != 0U) ? "emergency" : "normal");

    if (emergency_active != app->master_emergency_active)
    {
        app->master_emergency_active = emergency_active;
        (void)snprintf(buffer, sizeof(buffer), "[state] %s", (emergency_active != 0U) ? "emergency" : "normal");
        AppCore_SetResultText(app, buffer);

        if (emergency_active != 0U)
        {
            app->master_slave1_ok_pending = 0U;
            AppCore_SetButtonText(app, "waiting");
            if (AppCore_QueueCanCommandCode(app, APP_NODE_ID_SLAVE1, CAN_CMD_EMERGENCY, 0U) != 0U)
            {
                AppCore_SetResultText(app, "[can] emergency -> slave1");
            }
        }
    }

    if ((app->lin_last_reported_zone != status->zone) ||
        (app->lin_last_reported_lock != status->emergency_latched))
    {
        (void)snprintf(buffer,
                       sizeof(buffer),
                       "[lin] %s lock=%u adc=%u",
                       AppCore_GetLinZoneText(status->zone),
                       (unsigned int)status->emergency_latched,
                       (unsigned int)status->adc_value);
        AppCore_SetResultText(app, buffer);
        app->lin_last_reported_zone = status->zone;
        app->lin_last_reported_lock = status->emergency_latched;
    }

    if ((app->master_slave1_ok_pending != 0U) &&
        (status->zone == LIN_ZONE_EMERGENCY))
    {
        app->master_slave1_ok_pending = 0U;
        AppCore_SetButtonText(app, "cancelled");
        AppCore_SetLinLinkText(app, "emergency");
        AppCore_SetResultText(app, "[cancel] ok request cleared");
        return;
    }

    if ((app->master_slave1_ok_pending != 0U) &&
        (status->zone != LIN_ZONE_EMERGENCY) &&
        (status->emergency_latched == 0U))
    {
        if (AppCore_QueueCanCommandCode(app, APP_NODE_ID_SLAVE1, CAN_CMD_OK, 0U) != 0U)
        {
            app->master_slave1_ok_pending = 0U;
            AppCore_SetButtonText(app, "approved");
            AppCore_SetResultText(app, "[can] ok -> slave1");
        }
    }
}

/*
 * master가 수신한 역할 관련 CAN 명령을 처리한다.
 * 현재 중요한 입력은 slave1의 OK 요청이며,
 * 이것이 승인 판단 흐름을 시작시키는 계기가 된다.
 */
void AppMaster_HandleCanCommand(AppCore *app,
                                const CanMessage *message,
                                uint8_t *out_response_code)
{
    if ((app == NULL) || (message == NULL) || (out_response_code == NULL))
    {
        return;
    }

    if ((message->source_node_id == APP_NODE_ID_SLAVE1) &&
        (message->payload[0] == CAN_CMD_OK))
    {
        AppCore_SetCanInputText(app, "button in");
        AppCore_SetButtonText(app, "request");
        AppMaster_RequestOk(app);
        *out_response_code = CAN_RES_OK;
    }
}

/*
 * 승인 대기 중일 때 LIN OK-token 요청을 재시도한다.
 * master는 이 helper를 통해,
 * fresh status가 latch 해제를 확인할 때까지 slave2를 계속 자극한다.
 */
void AppMaster_AfterLinPoll(AppCore *app)
{
    LinStatusFrame status;

    if ((app == NULL) || (app->master_slave1_ok_pending == 0U))
    {
        return;
    }

    if (LinModule_GetLatestStatus(&app->lin_module, &status) == 0U)
    {
        return;
    }

    if ((status.zone != LIN_ZONE_EMERGENCY) && (status.emergency_latched != 0U))
    {
        (void)LinModule_RequestOk(&app->lin_module);
        AppCore_SetLinLinkText(app, "ok pending");
    }
}
