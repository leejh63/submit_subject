/*
 * master 전용 애플리케이션 오케스트레이션 구현부다.
 * coordinator에 필요한 console, CAN, LIN 경로만 연결하여,
 * 다른 노드 역할 분기 없이 master 흐름만 남긴다.
 */
#include "app_core.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "app_core_internal.h"
#include "app_master.h"
#include "../runtime/runtime_io.h"

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

static void AppCore_InitDefaultTexts(AppCore *app)
{
    if (app == NULL)
    {
        return;
    }

    AppCore_SetModeText(app, "normal");
    AppCore_SetButtonText(app, "waiting");
    AppCore_SetAdcText(app, "waiting");
    AppCore_SetCanInputText(app, "waiting");
    AppCore_SetLinInputText(app, "waiting");
    AppCore_SetLinLinkText(app, "waiting");
}

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
        (void)snprintf(buffer,
                       size,
                       "[timeout] %s target=%u",
                       name,
                       (unsigned int)result->source_node_id);
        return;
    }

    if (result->result_code == CAN_RES_OK)
    {
        (void)snprintf(buffer,
                       size,
                       "[ok] %s target=%u",
                       name,
                       (unsigned int)result->source_node_id);
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
    AppMaster_HandleCanCommand(app, message, &response_code);

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

InfraStatus AppCore_Init(AppCore *app)
{
    InfraStatus status;

    if (app == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(app, 0, sizeof(*app));
    app->local_node_id = RuntimeIo_GetLocalNodeId();
    app->lin_last_reported_zone = 0xFFU;
    app->lin_last_reported_lock = 0xFFU;
    AppCore_InitDefaultTexts(app);

    status = AppMaster_Init(app);
    if (status != INFRA_STATUS_OK)
    {
        return status;
    }

    app->initialized = 1U;
    return INFRA_STATUS_OK;
}

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
                (void)CanModule_QueueCommand(&app->can_module,
                                             command.target_node_id,
                                             CAN_CMD_OPEN,
                                             0U,
                                             0U,
                                             1U);
                activity = 1U;
                break;

            case APP_CONSOLE_CAN_CMD_CLOSE:
                (void)CanModule_QueueCommand(&app->can_module,
                                             command.target_node_id,
                                             CAN_CMD_CLOSE,
                                             0U,
                                             0U,
                                             1U);
                activity = 1U;
                break;

            case APP_CONSOLE_CAN_CMD_OFF:
                (void)CanModule_QueueCommand(&app->can_module,
                                             command.target_node_id,
                                             CAN_CMD_OFF,
                                             0U,
                                             0U,
                                             1U);
                activity = 1U;
                break;

            case APP_CONSOLE_CAN_CMD_TEST:
                (void)CanModule_QueueCommand(&app->can_module,
                                             command.target_node_id,
                                             CAN_CMD_TEST,
                                             0U,
                                             0U,
                                             1U);
                activity = 1U;
                break;

            case APP_CONSOLE_CAN_CMD_TEXT:
                (void)CanModule_QueueText(&app->can_module,
                                          command.target_node_id,
                                          command.text);
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

void AppCore_TaskLinFast(AppCore *app, uint32_t now_ms)
{
    LinStatusFrame status;

    if ((app == NULL) || (app->lin_enabled == 0U))
    {
        return;
    }

    LinModule_TaskFast(&app->lin_module, now_ms);
    if (LinModule_ConsumeFreshStatus(&app->lin_module, &status) != 0U)
    {
        AppMaster_HandleFreshLinStatus(app, &status);
    }
}

void AppCore_TaskLinPoll(AppCore *app, uint32_t now_ms)
{
    if (app == NULL)
    {
        return;
    }

    if (app->console_enabled != 0U)
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

    AppMaster_AfterLinPoll(app);
}

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
