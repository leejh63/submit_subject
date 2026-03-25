/*
 * LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다.
 * slave2에 필요한 ADC, LIN, LED task만 남겨,
 * 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다.
 */
#include "app_core.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "app_core_internal.h"
#include "app_slave2.h"
#include "../runtime/runtime_io.h"

static void AppCore_SetText(char *buffer, size_t size, const char *text)
{
    if ((buffer == NULL) || (size == 0U) || (text == NULL))
    {
        return;
    }

    (void)snprintf(buffer, size, "%s", text);
}

void AppCore_SetAdcText(AppCore *app, const char *text)
{
    if (app != NULL)
    {
        AppCore_SetText(app->adc_text, sizeof(app->adc_text), text);
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

static void AppCore_InitDefaultTexts(AppCore *app)
{
    if (app == NULL)
    {
        return;
    }

    AppCore_SetAdcText(app, "waiting");
    AppCore_SetLinInputText(app, "ready");
    AppCore_SetLinLinkText(app, "waiting");
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

    status = AppSlave2_Init(app);
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

void AppCore_TaskLinFast(AppCore *app, uint32_t now_ms)
{
    (void)now_ms;

    if ((app == NULL) || (app->lin_enabled == 0U))
    {
        return;
    }

    AppSlave2_HandleLinOkToken(app);
}

void AppCore_TaskLed(AppCore *app, uint32_t now_ms)
{
    if ((app == NULL) || (app->led2_enabled == 0U))
    {
        return;
    }

    LedModule_Task(&app->slave2_led, now_ms);
}

void AppCore_TaskAdc(AppCore *app, uint32_t now_ms)
{
    if ((app == NULL) || (app->adc_enabled == 0U))
    {
        return;
    }

    AppSlave2_TaskAdc(app, now_ms);
}
