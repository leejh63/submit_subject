/*
 * 모든 펌웨어 역할이 공유하는 애플리케이션 오케스트레이션 구현부다.
 * 역할별 policy 모듈을 초기화하고,
 * UART, CAN, LIN, ADC, LED 동작을 주기 task에 연결한다.
 */
#include "app_core.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "app/app_config.h"
#include "app/app_core_internal.h"
#include "app/app_master.h"
#include "app/app_slave1.h"
#include "app/app_slave2.h"
#include "runtime/runtime_io.h"

/*
 * AppCore UI 갱신 루틴이 공유하는 작은 텍스트 helper다.
 * snprintf 로직을 한곳에 모아두면,
 * 여러 view 필드에서 버퍼 처리 코드가 반복되지 않는다.
 */
static void AppCore_SetText(char *buffer, size_t size, const char *text)
{
    if ((buffer == NULL) || (size == 0U) || (text == NULL))
    {
        return;
    }

    (void)snprintf(buffer, size, "%s", text);
}

void AppCore_SetModeText(AppCore *app, const char *text)
{
    if (app != NULL)
    {
        AppCore_SetText(app->mode_text, sizeof(app->mode_text), text);
    }
}

void AppCore_SetButtonText(AppCore *app, const char *text)
{
    if (app != NULL)
    {
        AppCore_SetText(app->button_text, sizeof(app->button_text), text);
    }
}

void AppCore_SetAdcText(AppCore *app, const char *text)
{
    if (app != NULL)
    {
        AppCore_SetText(app->adc_text, sizeof(app->adc_text), text);
    }
}

void AppCore_SetCanInputText(AppCore *app, const char *text)
{
    if (app != NULL)
    {
        AppCore_SetText(app->can_input_text, sizeof(app->can_input_text), text);
    }
}

void AppCore_SetLinInputText(AppCore *app, const char *text)
{
    if (app != NULL)
    {
        AppCore_SetText(app->lin_input_text, sizeof(app->lin_input_text), text);
    }
}

void AppCore_SetLinLinkText(AppCore *app, const char *text)
{
    if (app != NULL)
    {
        AppCore_SetText(app->lin_link_text, sizeof(app->lin_link_text), text);
    }
}

void AppCore_SetResultText(AppCore *app, const char *text)
{
    if ((app != NULL) && (app->console_enabled != 0U))
    {
        AppConsole_SetResultText(&app->console, text);
    }
}

/*
 * LIN zone enum을 UI 친화적인 문자열로 바꾼다.
 * app 계층은 콘솔 출력과 event 보고용 상태 문자열을 만들 때,
 * 이 함수를 사용한다.
 */
const char *AppCore_GetLinZoneText(uint8_t zone)
{
    switch (zone)
    {
        case LIN_ZONE_SAFE:
            return "safe";

        case LIN_ZONE_WARNING:
            return "warning";

        case LIN_ZONE_DANGER:
            return "danger";

        case LIN_ZONE_EMERGENCY:
            return "emergency";

        default:
            return "unknown";
    }
}

/*
 * 콘솔 view를 역할별 기본 문자열로 초기화한다.
 * 실제 CAN, LIN, ADC, 버튼 동작이 들어오기 전에도,
 * 각 이미지가 의미 있는 startup 문자열을 보여주게 한다.
 */
static void AppCore_InitDefaultTexts(AppCore *app)
{
    if (app == NULL)
    {
        return;
    }

    if (app->role == APP_ROLE_MASTER)
    {
        AppCore_SetModeText(app, "normal");
        AppCore_SetButtonText(app, "waiting");
        AppCore_SetAdcText(app, "waiting");
        AppCore_SetCanInputText(app, "waiting");
        AppCore_SetLinInputText(app, "waiting");
        AppCore_SetLinLinkText(app, "waiting");
    }
    else if (app->role == APP_ROLE_SLAVE1)
    {
        AppCore_SetModeText(app, "normal");
        AppCore_SetButtonText(app, "ready");
        AppCore_SetAdcText(app, "n/a");
        AppCore_SetCanInputText(app, "waiting");
        AppCore_SetLinInputText(app, "n/a");
        AppCore_SetLinLinkText(app, "n/a");
    }
    else
    {
        AppCore_SetModeText(app, "monitor");
        AppCore_SetButtonText(app, "n/a");
        AppCore_SetAdcText(app, "waiting");
        AppCore_SetCanInputText(app, "n/a");
        AppCore_SetLinInputText(app, "ready");
        AppCore_SetLinLinkText(app, "waiting");
    }
}

/*
 * app 계층 CAN 제어 요청을 위한 편의 helper다.
 * 역할별 policy 코드는 전체 메시지를 다시 만들지 않고,
 * 이 wrapper로 공통 command 요청을 올릴 수 있다.
 */
uint8_t AppCore_QueueCanCommandCode(AppCore *app,
                                    uint8_t target_node_id,
                                    uint8_t command_code,
                                    uint8_t need_response)
{
    if ((app == NULL) || (app->can_enabled == 0U))
    {
        return 0U;
    }

    return (CanModule_QueueCommand(&app->can_module,
                                   target_node_id,
                                   command_code,
                                   0U,
                                   0U,
                                   need_response) == INFRA_STATUS_OK) ? 1U : 0U;
}

/*
 * 상호작용 역할에서 쓰는 콘솔과 CAN 모듈을 함께 올린다.
 * master와 slave1이 이 helper를 같이 사용하여,
 * 공통 통신 기능 startup 순서를 일관되게 유지한다.
 */
InfraStatus AppCore_InitConsoleCan(AppCore *app)
{
    CanModuleConfig can_config;

    if (app == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (AppConsole_Init(&app->console, app->local_node_id) == INFRA_STATUS_OK)
    {
        app->console_enabled = 1U;
    }

    (void)memset(&can_config, 0, sizeof(can_config));
    can_config.local_node_id = app->local_node_id;
    can_config.default_timeout_ms = 300U;
    can_config.max_submit_per_tick = 2U;
    if (CanModule_Init(&app->can_module, &can_config) != INFRA_STATUS_OK)
    {
        return INFRA_STATUS_IO_ERROR;
    }

    app->can_enabled = 1U;
    return INFRA_STATUS_OK;
}

/*
 * 낮은 수준의 CAN service 결과를 사람이 읽을 문자열로 바꾼다.
 * 콘솔은 이 문자열을 보여주어,
 * operator가 command/result 코드를 직접 해석하지 않아도 되게 한다.
 */
static void AppCore_FormatCanResult(const CanServiceResult *result, char *buffer, size_t size)
{
    const char *name;

    if ((result == NULL) || (buffer == NULL) || (size == 0U))
    {
        return;
    }

    switch (result->command_code)
    {
        case CAN_CMD_OPEN:
            name = "open";
            break;
        case CAN_CMD_CLOSE:
            name = "close";
            break;
        case CAN_CMD_OFF:
            name = "off";
            break;
        case CAN_CMD_TEST:
            name = "test";
            break;
        case CAN_CMD_OK:
            name = "ok";
            break;
        case CAN_CMD_EMERGENCY:
            name = "emergency";
            break;
        default:
            name = "unknown";
            break;
    }

    if (result->kind == CAN_SERVICE_RESULT_TIMEOUT)
    {
        (void)snprintf(buffer, size, "[timeout] %s target=%u", name, (unsigned int)result->source_node_id);
        return;
    }

    if (result->result_code == CAN_RES_OK)
    {
        (void)snprintf(buffer, size, "[ok] %s target=%u", name, (unsigned int)result->source_node_id);
        return;
    }

    if (result->result_code == CAN_RES_NOT_SUPPORTED)
    {
        (void)snprintf(buffer,
                       size,
                       "[error] %s not supported target=%u",
                       name,
                       (unsigned int)result->source_node_id);
        return;
    }

    (void)snprintf(buffer,
                   size,
                   "[error] %s target=%u code=%u",
                   name,
                   (unsigned int)result->source_node_id,
                   (unsigned int)result->result_code);
}

/*
 * decode된 CAN 메시지 하나를 현재 역할 policy로 전달한다.
 * response는 CAN service에 남기고,
 * command와 event는 여기서 UI 갱신과 policy callback에 사용한다.
 */
static void AppCore_HandleCanIncoming(AppCore *app, const CanMessage *message)
{
    char    buffer[APP_CONSOLE_RESULT_VIEW_SIZE];
    uint8_t response_code;

    if ((app == NULL) || (message == NULL))
    {
        return;
    }

    if (message->message_type == CAN_MSG_EVENT)
    {
        (void)snprintf(buffer,
                       sizeof(buffer),
                       "[event] code=%u from=%u arg0=%u arg1=%u",
                       (unsigned int)message->payload[0],
                       (unsigned int)message->source_node_id,
                       (unsigned int)message->payload[1],
                       (unsigned int)message->payload[2]);
        AppCore_SetResultText(app, buffer);
        return;
    }

    if (message->message_type == CAN_MSG_TEXT)
    {
        (void)snprintf(buffer,
                       sizeof(buffer),
                       "[text] from=%u %s",
                       (unsigned int)message->source_node_id,
                       message->text);
        AppCore_SetResultText(app, buffer);
        return;
    }

    if (message->message_type != CAN_MSG_COMMAND)
    {
        return;
    }

    response_code = CAN_RES_NOT_SUPPORTED;
    if (app->role == APP_ROLE_SLAVE1)
    {
        AppSlave1_HandleCanCommand(app, message, &response_code);
    }
    else if (app->role == APP_ROLE_MASTER)
    {
        AppMaster_HandleCanCommand(app, message, &response_code);
    }

    (void)snprintf(buffer,
                   sizeof(buffer),
                   "[remote] cmd=%u from=%u",
                   (unsigned int)message->payload[0],
                   (unsigned int)message->source_node_id);
    AppCore_SetResultText(app, buffer);

    if ((message->flags & CAN_MSG_FLAG_NEED_RESPONSE) != 0U)
    {
        (void)CanModule_QueueResponse(&app->can_module,
                                      message->source_node_id,
                                      message->request_id,
                                      response_code,
                                      0U);
    }
}

/*
 * 현재 역할에 맞는 공통 application context를 초기화한다.
 * 상태를 비우고 로컬 identity를 기록한 뒤,
 * 더 깊은 startup은 선택된 policy 모듈에 위임한다.
 */
InfraStatus AppCore_Init(AppCore *app)
{
    InfraStatus status;

    if (app == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(app, 0, sizeof(*app));
    app->role = RuntimeIo_GetActiveRole();
    app->local_node_id = RuntimeIo_GetLocalNodeId();
    app->lin_last_reported_zone = 0xFFU;
    app->lin_last_reported_lock = 0xFFU;
    AppCore_InitDefaultTexts(app);

    status = INFRA_STATUS_UNSUPPORTED;
    if (app->role == APP_ROLE_MASTER)
    {
        status = AppMaster_Init(app);
    }
    else if (app->role == APP_ROLE_SLAVE1)
    {
        status = AppSlave1_Init(app);
    }
    else if (app->role == APP_ROLE_SLAVE2)
    {
        status = AppSlave2_Init(app);
    }

    if (status != INFRA_STATUS_OK)
    {
        return status;
    }

    app->initialized = 1U;
    return INFRA_STATUS_OK;
}

/*
 * base timer interrupt 경로에서 호출되는 tick hook이다.
 * 현재는 LIN 기능이 켜진 역할에 대해,
 * timing service를 LIN 모듈로 전달하는 역할을 한다.
 */
void AppCore_OnTickIsr(void *context)
{
    AppCore *app;

    app = (AppCore *)context;
    if ((app == NULL) || (app->lin_enabled == 0U))
    {
        return;
    }

    LinModule_OnBaseTick(&app->lin_module);
}

void AppCore_TaskHeartbeat(AppCore *app, uint32_t now_ms)
{
    (void)now_ms;

    if (app != NULL)
    {
        app->heartbeat_count++;
    }
}

void AppCore_TaskUart(AppCore *app, uint32_t now_ms)
{
    if ((app == NULL) || (app->console_enabled == 0U))
    {
        return;
    }

    app->uart_task_count++;
    AppConsole_Task(&app->console, now_ms);
}

/*
 * queue에 쌓인 작업과 새 트래픽을 처리하는 주기 CAN task다.
 * 콘솔 요청을 비우고 CAN 모듈을 진행시킨 다음,
 * 결과와 수신 메시지를 app 계층에 전달한다.
 */
void AppCore_TaskCan(AppCore *app, uint32_t now_ms)
{
    AppConsoleCanCommand command;
    CanServiceResult     result;
    CanMessage           message;
    char                 buffer[APP_CONSOLE_RESULT_VIEW_SIZE];
    uint8_t              activity;

    if ((app == NULL) || (app->can_enabled == 0U))
    {
        return;
    }

    activity = 0U;
    while (app->console_enabled != 0U && AppConsole_TryPopCanCommand(&app->console, &command) != 0U)
    {
        switch (command.type)
        {
            case APP_CONSOLE_CAN_CMD_OPEN:
                (void)CanModule_QueueCommand(&app->can_module, command.target_node_id, CAN_CMD_OPEN, 0U, 0U, 1U);
                activity = 1U;
                break;
            case APP_CONSOLE_CAN_CMD_CLOSE:
                (void)CanModule_QueueCommand(&app->can_module, command.target_node_id, CAN_CMD_CLOSE, 0U, 0U, 1U);
                activity = 1U;
                break;
            case APP_CONSOLE_CAN_CMD_OFF:
                (void)CanModule_QueueCommand(&app->can_module, command.target_node_id, CAN_CMD_OFF, 0U, 0U, 1U);
                activity = 1U;
                break;
            case APP_CONSOLE_CAN_CMD_TEST:
                (void)CanModule_QueueCommand(&app->can_module, command.target_node_id, CAN_CMD_TEST, 0U, 0U, 1U);
                activity = 1U;
                break;
            case APP_CONSOLE_CAN_CMD_TEXT:
                (void)CanModule_QueueText(&app->can_module, command.target_node_id, command.text);
                activity = 1U;
                break;
            case APP_CONSOLE_CAN_CMD_EVENT:
                (void)CanModule_QueueEvent(&app->can_module,
                                           command.target_node_id,
                                           command.event_code,
                                           command.arg0,
                                           command.arg1);
                activity = 1U;
                break;
            default:
                break;
        }
    }

    CanModule_Task(&app->can_module, now_ms);
    while (CanModule_TryPopResult(&app->can_module, &result) != 0U)
    {
        AppCore_FormatCanResult(&result, buffer, sizeof(buffer));
        AppCore_SetResultText(app, buffer);
        activity = 1U;
    }

    while (CanModule_TryPopIncoming(&app->can_module, &message) != 0U)
    {
        AppCore_HandleCanIncoming(app, &message);
        activity = 1U;
    }

    app->can_last_activity = activity;
    if (activity != 0U)
    {
        app->can_task_count++;
    }
}

/*
 * 상태기계 진행과 새 데이터 수신을 위한 빠른 LIN task다.
 * master는 여기서 새 센서 상태를 소비하고,
 * slave2는 coordinator의 승인 token 도착 여부를 확인한다.
 */
void AppCore_TaskLinFast(AppCore *app, uint32_t now_ms)
{
    LinStatusFrame status;

    if ((app == NULL) || (app->lin_enabled == 0U))
    {
        return;
    }

    LinModule_TaskFast(&app->lin_module, now_ms);

    if ((app->role == APP_ROLE_MASTER) &&
        (LinModule_ConsumeFreshStatus(&app->lin_module, &status) != 0U))
    {
        AppMaster_HandleFreshLinStatus(app, &status);
    }

    if (app->role == APP_ROLE_SLAVE2)
    {
        AppSlave2_HandleLinOkToken(app);
    }
}

/*
 * 예약된 poll과 승인 작업을 처리하는 느린 LIN task다.
 * master는 이 주기에서 status poll을 보내고,
 * slave1 승인 대기 중에는 OK-token 경로를 재시도한다.
 */
void AppCore_TaskLinPoll(AppCore *app, uint32_t now_ms)
{
    if (app == NULL)
    {
        return;
    }

    if ((app->role == APP_ROLE_MASTER) && (app->console_enabled != 0U))
    {
        if (AppConsole_ConsumeLocalOk(&app->console) != 0U)
        {
            AppMaster_RequestOk(app);
        }
    }

    if (app->lin_enabled != 0U)
    {
        LinModule_TaskPoll(&app->lin_module, now_ms);
    }

    if (app->role == APP_ROLE_MASTER)
    {
        AppMaster_AfterLinPoll(app);
    }
}

void AppCore_TaskButton(AppCore *app, uint32_t now_ms)
{
    if ((app == NULL) || (app->role != APP_ROLE_SLAVE1))
    {
        return;
    }

    AppSlave1_TaskButton(app, now_ms);
}

void AppCore_TaskLed(AppCore *app, uint32_t now_ms)
{
    if (app == NULL)
    {
        return;
    }

    if (app->role == APP_ROLE_SLAVE1)
    {
        AppSlave1_TaskLed(app, now_ms);
    }

    if (app->led2_enabled != 0U)
    {
        LedModule_Task(&app->slave2_led, now_ms);
    }
}

void AppCore_TaskAdc(AppCore *app, uint32_t now_ms)
{
    if ((app == NULL) || (app->role != APP_ROLE_SLAVE2))
    {
        return;
    }

    AppSlave2_TaskAdc(app, now_ms);
}

/*
 * 최신 app 상태를 바탕으로 UART 콘솔 view를 갱신한다.
 * 렌더 문자열을 여기서 조합해 두면,
 * console 모듈은 transport와 layout, dirty-line 갱신에만 집중할 수 있다.
 */
void AppCore_TaskRender(AppCore *app, uint32_t now_ms)
{
    char        task_text[APP_CONSOLE_TASK_VIEW_SIZE];
    char        source_text[APP_CONSOLE_SOURCE_VIEW_SIZE];
    char        value_text[APP_CONSOLE_VALUE_VIEW_SIZE];
    const char *can_text;
    const char *uart_text;

    (void)now_ms;

    if ((app == NULL) || (app->console_enabled == 0U))
    {
        return;
    }

    can_text = (app->can_last_activity != 0U) ? "ok" : "idle";
    uart_text = (UartService_HasError(&app->console.uart) == 0U) ? "ok" : "error";

    (void)snprintf(task_text,
                   sizeof(task_text),
                   "HeartBeat : alive / %lu\r\n"
                   "CAN       : %s / %lu\r\n"
                   "LIN       : %s\r\n"
                   "UART      : %s / %lu",
                   (unsigned long)app->heartbeat_count,
                   can_text,
                   (unsigned long)app->can_task_count,
                   app->lin_link_text,
                   uart_text,
                   (unsigned long)app->uart_task_count);

    (void)snprintf(source_text,
                   sizeof(source_text),
                   "from [can] \"%s\"\r\n"
                   "from [lin] \"%s\"",
                   app->can_input_text,
                   app->lin_input_text);

    (void)snprintf(value_text,
                   sizeof(value_text),
                   "Mode   : %s\r\n"
                   "Button : %s\r\n"
                   "ADC    : %s",
                   app->mode_text,
                   app->button_text,
                   app->adc_text);

    AppConsole_SetTaskText(&app->console, task_text);
    AppConsole_SetSourceText(&app->console, source_text);
    AppConsole_SetValueText(&app->console, value_text);
    AppConsole_Render(&app->console);
}
