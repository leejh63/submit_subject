// master 노드 정책을 구현한 파일이다.
// LIN 상태를 기준으로 emergency 여부를 판정하고,
// slave1 통지 시점과 slave2 latch 해제 승인 흐름을 조정한다.
#include "app_master.h"

#include <stddef.h>
#include <stdio.h>

#include "app_config.h"
#include "app_core_internal.h"
#include "../runtime/runtime_io.h"

// slave2 승인 결과를 slave1 통지로 연결하는 relay 절차가 진행 중인지 확인한다.
static uint8_t AppMaster_IsOkRelayActive(const AppCore *app)
{
    if (app == NULL)
    {
        return 0U;
    }

    return (app->ok_relay.state != APP_MASTER_OK_RELAY_IDLE) ? 1U : 0U;
}

// OK relay 추적 상태를 초기값으로 복원한다.
static void AppMaster_ResetOkRelay(AppCore *app)
{
    if (app == NULL)
    {
        return;
    }

    app->ok_relay.state = APP_MASTER_OK_RELAY_IDLE;
    app->ok_relay.retry_count = 0U;
    app->ok_relay.started_ms = 0U;
    app->ok_relay.last_retry_ms = 0U;
}

// slave2 latch 해제 대기 상태를 시작한다.
// 시작 시각과 마지막 재시도 시각을 함께 기록하여,
// 이후 poll 단계에서 timeout과 재시도 간격을 계산할 수 있게 한다.
static void AppMaster_StartOkRelay(AppCore *app, uint32_t now_ms)
{
    if (app == NULL)
    {
        return;
    }

    app->ok_relay.state = APP_MASTER_OK_RELAY_WAIT_SLAVE_CLEAR;
    app->ok_relay.retry_count = 1U;
    app->ok_relay.started_ms = now_ms;
    app->ok_relay.last_retry_ms = now_ms;
}

// 진행 중인 OK relay 절차를 취소하고 관련 UI 문구를 함께 정리한다.
static void AppMaster_CancelOkRelay(AppCore *app,
                                    const char *button_text,
                                    const char *link_text,
                                    const char *result_text)
{
    if (app == NULL)
    {
        return;
    }

    AppMaster_ResetOkRelay(app);
    if (button_text != NULL)
    {
        AppCore_SetButtonText(app, button_text);
    }
    if (link_text != NULL)
    {
        AppCore_SetLinLinkText(app, link_text);
    }
    if (result_text != NULL)
    {
        AppCore_SetResultText(app, result_text);
    }
}

typedef enum
{
    APP_MASTER_RESULT_PRIORITY_NONE = 0,
    APP_MASTER_RESULT_PRIORITY_LIN,
    APP_MASTER_RESULT_PRIORITY_STATE,
    APP_MASTER_RESULT_PRIORITY_RELAY
} AppMasterResultPriority;

typedef enum
{
    APP_MASTER_RELAY_RESULT_NONE = 0,
    APP_MASTER_RELAY_RESULT_CANCELLED,
    APP_MASTER_RELAY_RESULT_APPROVED,
    APP_MASTER_RELAY_RESULT_APPROVE_BUSY
} AppMasterRelayResult;

// 한 번의 상태 처리에서 어떤 결과 문구를 남길지 우선순위를 정한다.
static void AppMaster_SelectResultText(const char **selected_result_text,
                                       uint8_t *selected_priority,
                                       const char *candidate_text,
                                       uint8_t candidate_priority)
{
    if ((selected_result_text == NULL) || (selected_priority == NULL) || (candidate_text == NULL))
    {
        return;
    }

    if (candidate_priority >= *selected_priority)
    {
        *selected_result_text = candidate_text;
        *selected_priority = candidate_priority;
    }
}

// slave2 latch 해제가 확인된 이후 slave1 승인 CAN 명령을 큐에 적재한다.
// 실제 승인 조건 확인은 호출자 측에서 완료된 상태를 전제로 한다.
static AppMasterRelayResult AppMaster_TryApproveSlave1(AppCore *app, const char **out_result_text)
{
    const char *result_text;

    if (app == NULL)
    {
        return APP_MASTER_RELAY_RESULT_NONE;
    }

    result_text = NULL;
    if (AppCore_QueueCanCommandCode(app, APP_NODE_ID_SLAVE1, CAN_CMD_OK, 0U) == 0U)
    {
        AppCore_SetLinLinkText(app, "can busy");
        result_text = "[busy] can ok queue full";
        if (out_result_text != NULL)
        {
            *out_result_text = result_text;
        }
        else
        {
            AppCore_SetResultText(app, result_text);
        }
        return APP_MASTER_RELAY_RESULT_APPROVE_BUSY;
    }

    AppMaster_ResetOkRelay(app);
    AppCore_SetButtonText(app, "approved");
    AppCore_SetLinLinkText(app, "clear");
    result_text = "[can] ok -> slave1";
    if (out_result_text != NULL)
    {
        *out_result_text = result_text;
    }
    else
    {
        AppCore_SetResultText(app, result_text);
    }
    return APP_MASTER_RELAY_RESULT_APPROVED;
}

// 현재 LIN 상태를 fail-closed 기준으로 emergency로 간주해야 하는지 판정한다.
static uint8_t AppMaster_IsFaultOrEmergencyStatus(const LinStatusFrame *status)
{
    if (status == NULL)
    {
        return 1U;
    }

    if (status->fault != 0U)
    {
        return 1U;
    }

    if (status->zone == LIN_ZONE_EMERGENCY)
    {
        return 1U;
    }

    return (status->emergency_latched != 0U) ? 1U : 0U;
}

// fault 여부와 relay 진행 상태를 반영해 LIN 관련 UI 문자열을 갱신한다.
// 이후 정책 처리에서 link/button 표시는 다시 조정될 수 있으므로,
// 여기서는 기본 상태를 맞추는 1차 표시 동기화에 집중한다.
static void AppMaster_UpdateLinStatusUi(AppCore *app,
                                        const LinStatusFrame *status,
                                        uint8_t ok_relay_active,
                                        const char *zone_text)
{
    char buffer[APP_CONSOLE_RESULT_VIEW_SIZE];

    if ((app == NULL) || (status == NULL))
    {
        return;
    }

    if (status->fault != 0U)
    {
        AppCore_SetAdcText(app, "fault (fail closed)");
        AppCore_SetLinInputText(app, "adc in : fault");
        AppCore_SetLinLinkText(app, "fault");
        return;
    }

    (void)snprintf(buffer,
                   sizeof(buffer),
                   "%u (%s, lock=%u)",
                   (unsigned int)status->adc_value,
                   zone_text,
                   (unsigned int)status->emergency_latched);
    AppCore_SetAdcText(app, buffer);

    (void)snprintf(buffer,
                   sizeof(buffer),
                   "adc in : %u (%s)",
                   (unsigned int)status->adc_value,
                   zone_text);
    AppCore_SetLinInputText(app, buffer);

    if ((ok_relay_active != 0U) &&
        (status->zone != LIN_ZONE_EMERGENCY) &&
        (status->emergency_latched != 0U))
    {
        AppCore_SetLinLinkText(app, "release pending");
    }
    else
    {
        AppCore_SetLinLinkText(app, "idle");
    }
}

// emergency 진입 시 진행 중인 relay를 정리하고, 버튼/링크 UI를 차단 상태로 맞춘다.
static void AppMaster_blockOkRelayForEmergency(AppCore *app, uint8_t is_fault)
{
    if (app == NULL)
    {
        return;
    }

    AppMaster_CancelOkRelay(app,
                            "blocked",
                            (is_fault != 0U) ? "fault" : "emergency",
                            NULL);
}

// 최신 LIN 상태에 따라 relay 대기 흐름을 계속할지, 취소할지, 승인할지를 결정한다.
static AppMasterRelayResult AppMaster_HandleOkRelayWithLinStatus(AppCore *app,
                                                                 const LinStatusFrame *status,
                                                                 const char **out_result_text)
{
    if ((app == NULL) || (status == NULL) || (AppMaster_IsOkRelayActive(app) == 0U))
    {
        return APP_MASTER_RELAY_RESULT_NONE;
    }

    if (status->fault != 0U)
    {
        AppMaster_CancelOkRelay(app, "cancelled", "fault", NULL);
        if (out_result_text != NULL)
        {
            *out_result_text = "[cancel] lin fault";
        }
        else
        {
            AppCore_SetResultText(app, "[cancel] lin fault");
        }
        return APP_MASTER_RELAY_RESULT_CANCELLED;
    }

    if (status->zone == LIN_ZONE_EMERGENCY)
    {
        AppMaster_CancelOkRelay(app, "cancelled", "emergency", NULL);
        if (out_result_text != NULL)
        {
            *out_result_text = "[cancel] ok relay aborted by emergency";
        }
        else
        {
            AppCore_SetResultText(app, "[cancel] ok relay aborted by emergency");
        }
        return APP_MASTER_RELAY_RESULT_CANCELLED;
    }

    if (status->emergency_latched == 0U)
    {
        return AppMaster_TryApproveSlave1(app, out_result_text);
    }

    return APP_MASTER_RELAY_RESULT_NONE;
}

// master 역할에 필요한 통신 모듈을 초기화한다.
// 이 노드는 console, CAN, LIN 경로를 모두 사용하므로,
// 초기화 단계에서 공통 통신 자원을 우선 준비한다.
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

    if (RuntimeIo_GetMasterLinConfig(&lin_config) == INFRA_STATUS_OK)
    {
        RuntimeIo_AttachLinModule(&app->lin_module);
        if (LinModule_Init(&app->lin_module, &lin_config) == INFRA_STATUS_OK)
        {
            app->lin_enabled = 1U;
        }
        else
        {
            AppCore_SetLinLinkText(app, "binding req");
        }
    }
    else
    {
        AppCore_SetLinLinkText(app, "binding req");
    }

    return INFRA_STATUS_OK;
}

// slave1에서 들어온 승인 요청을 처리한다.
// master는 최신 LIN 상태가 emergency 구간을 벗어난 경우에만,
// OK token 요청을 slave2로 전달한다.
void AppMaster_RequestOk(AppCore *app, uint32_t now_ms)
{
    LinStatusFrame status;
    InfraStatus    status_ready;
    InfraStatus    request_status;

    if ((app == NULL) || (app->lin_enabled == 0U))
    {
        AppCore_SetResultText(app, "[wait] lin not ready");
        return;
    }

    if (AppMaster_IsOkRelayActive(app) != 0U)
    {
        AppCore_SetButtonText(app, "busy");
        AppCore_SetResultText(app, "[busy] ok already pending");
        return;
    }

    status_ready = LinModule_GetLatestStatusIfFresh(&app->lin_module,
                                                    now_ms,
                                                    APP_LIN_STATUS_MAX_AGE_MS,
                                                    &status);
    if (status_ready == INFRA_STATUS_EMPTY)
    {
        AppCore_SetButtonText(app, "waiting lin");
        AppCore_SetResultText(app, "[wait] no lin status");
        return;
    }

    if (status_ready != INFRA_STATUS_OK)
    {
        AppCore_SetButtonText(app, "stale");
        AppCore_SetLinLinkText(app, "stale");
        AppCore_SetResultText(app, "[wait] lin status stale");
        return;
    }

    if (status.fault != 0U)
    {
        AppCore_SetButtonText(app, "sensor fault");
        AppCore_SetLinLinkText(app, "fault");
        AppCore_SetResultText(app, "[deny] lin sensor fault");
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

    request_status = LinModule_RequestOk(&app->lin_module);
    if (request_status != INFRA_STATUS_OK)
    {
        AppCore_SetButtonText(app, "busy");
        AppCore_SetLinLinkText(app, "busy");
        AppCore_SetResultText(app, "[busy] lin ok tx busy");
        return;
    }

    AppMaster_StartOkRelay(app, now_ms);
    AppCore_SetButtonText(app, "ok queued");
    AppCore_SetLinLinkText(app, "ok pending");
    AppCore_SetResultText(app, "[queued] slave1 ok -> lin ok");
}

// 새로 수신한 LIN status를 기준으로 UI, 상태 전이, relay 판단을 반영한다.
// TEMP: 상태 전이 처리 / emergency 반응 / relay 판단 / result 선택은 이후 분리 필요
void AppMaster_HandleFreshLinStatus(AppCore *app, const LinStatusFrame *status)
{
    AppMasterRelayResult relay_result;
    const char *zone_text;
    const char *result_text;
    const char *relay_result_text;
    uint8_t     result_priority;
    uint8_t     is_fault;
    uint8_t     ui_ok_relay_active;
    uint8_t     emergency_active;
    char        state_buffer[APP_CONSOLE_RESULT_VIEW_SIZE];
    char        lin_buffer[APP_CONSOLE_RESULT_VIEW_SIZE];

    if ((app == NULL) || (status == NULL))
    {
        return;
    }

    // 현재 LIN status를 해석하고 UI 갱신에 필요한 기본 값을 준비한다.
    zone_text = AppCore_GetLinZoneText(status->zone);
    result_text = NULL;
    relay_result_text = NULL;
    relay_result = APP_MASTER_RELAY_RESULT_NONE;
    result_priority = APP_MASTER_RESULT_PRIORITY_NONE;
    is_fault = (status->fault != 0U) ? 1U : 0U;
    ui_ok_relay_active = AppMaster_IsOkRelayActive(app);

    // 현재 LIN 상태를 화면에 반영한다.
    AppMaster_UpdateLinStatusUi(app, status, ui_ok_relay_active, zone_text);

    // fault / emergency / normal 상태를 판정하고 mode 표시를 갱신한다.
    emergency_active = AppMaster_IsFaultOrEmergencyStatus(status);
    if (is_fault != 0U)
    {
        AppCore_SetModeText(app, "fault");
    }
    else
    {
        AppCore_SetModeText(app, (emergency_active != 0U) ? "emergency" : "normal");
    }

    // emergency 상태 전이가 발생했는지 확인하고 필요한 반응을 처리한다.
    if (emergency_active != app->master_block_active)
    {
        app->master_block_active = emergency_active;
        (void)snprintf(state_buffer,
                       sizeof(state_buffer),
                       "[state] %s",
                       (is_fault != 0U) ? "fault" : ((emergency_active != 0U) ? "emergency" : "normal"));
        AppMaster_SelectResultText(&result_text,
                                   &result_priority,
                                   state_buffer,
                                   APP_MASTER_RESULT_PRIORITY_STATE);

        // emergency 진입 시 relay를 차단하고 slave1에 현재 상태를 알린다.
        if (emergency_active != 0U)
        {
            AppMaster_blockOkRelayForEmergency(app, is_fault);
            if (AppCore_QueueCanCommandCode(app, APP_NODE_ID_SLAVE1, CAN_CMD_EMERGENCY, 0U) != 0U)
            {
                AppMaster_SelectResultText(&result_text,
                                           &result_priority,
                                           (is_fault != 0U) ? "[can] fault -> slave1" : "[can] emergency -> slave1",
                                           APP_MASTER_RESULT_PRIORITY_RELAY);
            }
        }
    }

    // 마지막으로 보고한 LIN 상태와 비교해 변화가 있을 때만 결과 후보를 갱신한다.
    if ((app->lin_last_reported_zone != status->zone) ||
        (app->lin_last_reported_latch != status->emergency_latched) ||
        (app->lin_last_reported_fault != status->fault))
    {
        if (is_fault != 0U)
        {
            (void)snprintf(lin_buffer,
                           sizeof(lin_buffer),
                           "[lin] fault lock=%u adc=%u",
                           (unsigned int)status->emergency_latched,
                           (unsigned int)status->adc_value);
        }
        else
        {
            (void)snprintf(lin_buffer,
                           sizeof(lin_buffer),
                           "[lin] %s lock=%u adc=%u",
                           zone_text,
                           (unsigned int)status->emergency_latched,
                           (unsigned int)status->adc_value);
        }
        AppMaster_SelectResultText(&result_text,
                                   &result_priority,
                                   lin_buffer,
                                   APP_MASTER_RESULT_PRIORITY_LIN);
        app->lin_last_reported_zone = status->zone;
        app->lin_last_reported_latch = status->emergency_latched;
        app->lin_last_reported_fault = status->fault;
    }

    // 진행 중인 OK relay를 현재 status로 평가한다.
    relay_result = AppMaster_HandleOkRelayWithLinStatus(app, status, &relay_result_text);
    if (relay_result != APP_MASTER_RELAY_RESULT_NONE)
    {
        AppMaster_SelectResultText(&result_text,
                                   &result_priority,
                                   relay_result_text,
                                   APP_MASTER_RESULT_PRIORITY_RELAY);
    }

    // 우선순위가 가장 높은 결과 메시지를 최종 반영한다.
    if (result_text != NULL)
    {
        AppCore_SetResultText(app, result_text);
    }
}

// master가 수신한 역할 관련 CAN 명령을 처리한다.
// 현재는 slave1이 보낸 OK 요청만 정책 입력으로 사용하며,
// 해당 명령이 승인 판단 절차의 시작점이 된다.
void AppMaster_HandleCanCommand(AppCore *app,
                                const CanMessage *message,
                                uint32_t now_ms,
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
        AppMaster_RequestOk(app, now_ms);
        *out_response_code = CAN_RES_OK;
    }
}

// 승인 대기 중인 relay 흐름을 주기적으로 점검한다.
// latch 해제가 아직 확인되지 않으면 LIN OK 요청을 재시도하고,
// fault, stale, timeout, retry 한계 조건에서는 취소 경로로 정리한다.
void AppMaster_AfterLinPoll(AppCore *app, uint32_t now_ms)
{
    AppMasterRelayResult relay_result;
    LinStatusFrame status;
    InfraStatus    status_ready;
    InfraStatus    request_status;

    // 앱이 없거나 현재 진행 중인 OK relay가 없으면 더 볼 필요가 없다.
    if ((app == NULL) || (AppMaster_IsOkRelayActive(app) == 0U))
    {
        return;
    }

    // 최신 LIN status가 아직 유효한지 확인하고, fresh 상태면 relay 완료/취소 조건을 먼저 평가한다.
    status_ready = LinModule_GetLatestStatusIfFresh(&app->lin_module,
                                                    now_ms,
                                                    APP_LIN_STATUS_MAX_AGE_MS,
                                                    &status);
    if (status_ready == INFRA_STATUS_OK)
    {
        const char *relay_result_text;

        relay_result_text = NULL;
        relay_result = AppMaster_HandleOkRelayWithLinStatus(app, &status, &relay_result_text);
        if (relay_result != APP_MASTER_RELAY_RESULT_NONE)
        {
            // fresh status만으로 relay 결과가 결정되면 즉시 UI에 반영하고 재시도 흐름을 종료한다.
            if (relay_result_text != NULL)
            {
                AppCore_SetResultText(app, relay_result_text);
            }
            return;
        }
    }

    // relay 시작 후 전체 허용 시간을 넘겼으면 더 이상 기다리지 않고 timeout으로 정리한다.
    if (Infra_TimeIsExpired(now_ms, app->ok_relay.started_ms, APP_MASTER_OK_TIMEOUT_MS) != 0U)
    {
        AppMaster_CancelOkRelay(app, "timeout", "timeout", "[timeout] lin ok response missing");
        return;
    }

    // 최신 status를 신뢰할 수 없는 상태면 링크 상태를 stale로 표시한다.
    // EMPTY는 아직 새 데이터가 안 온 경우로 보고, 그 외 오류만 stale 취급한다.
    if ((status_ready != INFRA_STATUS_OK) && (status_ready != INFRA_STATUS_EMPTY))
    {
        AppCore_SetLinLinkText(app, "stale");
    }

    // 재시도 횟수 한계를 넘기면 relay를 더 끌지 않고 timeout 계열로 종료한다.
    if (app->ok_relay.retry_count >= APP_MASTER_OK_MAX_RETRY_COUNT)
    {
        AppMaster_CancelOkRelay(app, "timeout", "retry max", "[timeout] lin ok retry limit");
        return;
    }

    // 마지막 재시도 이후 아직 간격이 충분히 지나지 않았으면 이번 poll에서는 아무 것도 하지 않는다.
    if (Infra_TimeIsDue(now_ms,
                        app->ok_relay.last_retry_ms,
                        APP_MASTER_OK_RETRY_INTERVAL_MS) == 0U)
    {
        return;
    }

    // 재시도 시점이 되었으면 slave2에 LIN OK 요청을 다시 넣어 latch 해제를 유도한다.
    request_status = LinModule_RequestOk(&app->lin_module);
    if (request_status == INFRA_STATUS_OK)
    {
        // 요청이 큐잉되면 retry 정보를 갱신하고, 현재 승인 대기 중임을 UI에 표시한다.
        app->ok_relay.retry_count++;
        app->ok_relay.last_retry_ms = now_ms;
        AppCore_SetLinLinkText(app, "ok pending");
        AppCore_SetResultText(app, "[retry] lin ok request");
    }
}
