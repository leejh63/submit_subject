/*
 * CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다.
 * slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨,
 * UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다.
 */
#include "app_core.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "app_core_internal.h"
#include "app_slave1.h"
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

void AppCore_SetResultText(AppCore *app, const char *text)
{
    (void)app;
    (void)text;
}

static void AppCore_InitDefaultTexts(AppCore *app)
{
    if (app == NULL)
    {
        return;
    }

    AppCore_SetModeText(app, "normal");
    AppCore_SetButtonText(app, "ready");
    AppCore_SetAdcText(app, "n/a");
    AppCore_SetCanInputText(app, "waiting");
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

static void AppCore_HandleCanIncoming(AppCore *app, const CanMessage *message)
{
    uint8_t response_code;

    if ((app == NULL) || (message == NULL))
    {
        return;
    }

    if (message->message_type != CAN_MSG_COMMAND)
    {
        return;
    }

    response_code = CAN_RES_NOT_SUPPORTED;
    AppSlave1_HandleCanCommand(app, message, &response_code);

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
    AppCore_InitDefaultTexts(app);

    status = AppSlave1_Init(app);
    if (status != INFRA_STATUS_OK)
    {
        return status;
    }

    app->initialized = 1U;
    return INFRA_STATUS_OK;
}

void AppCore_TaskHeartbeat(AppCore *app, uint32_t now_ms)
{
    (void)now_ms;

    if (app != NULL)
    {
        app->heartbeat_count++;
    }
}

void AppCore_TaskCan(AppCore *app, uint32_t now_ms)
{
    CanServiceResult result;
    CanMessage       message;

    if ((app == NULL) || (app->can_enabled == 0U))
    {
        return;
    }

    CanModule_Task(&app->can_module, now_ms);

    while (CanModule_TryPopResult(&app->can_module, &result) != 0U)
    {
        (void)result;
    }

    while (CanModule_TryPopIncoming(&app->can_module, &message) != 0U)
    {
        AppCore_HandleCanIncoming(app, &message);
    }
}

void AppCore_TaskButton(AppCore *app, uint32_t now_ms)
{
    if (app == NULL)
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

    AppSlave1_TaskLed(app, now_ms);
}
