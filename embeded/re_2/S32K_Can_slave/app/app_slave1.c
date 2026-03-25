/*
 * slave1 policy 구현부다.
 * CAN 명령을 LED 동작으로 바꾸고,
 * 안정된 버튼 입력을 master로 보내는 승인 요청으로 변환한다.
 */
#include "app_slave1.h"

#include <stddef.h>
#include <stdio.h>

#include "app/app_config.h"
#include "app/app_core_internal.h"
#include "runtime/runtime_io.h"

/*
 * 현장 반응 노드 역할을 초기화한다.
 * slave1은 공통 console과 CAN 경로,
 * 그리고 emergency/approval 표시를 위한 로컬 LED 제어기가 필요하다.
 */
InfraStatus AppSlave1_Init(AppCore *app)
{
    LedConfig led_config;

    if (app == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (AppCore_InitConsoleCan(app) != INFRA_STATUS_OK)
    {
        return INFRA_STATUS_IO_ERROR;
    }

    if ((RuntimeIo_GetSlave1LedConfig(&led_config) == INFRA_STATUS_OK) &&
        (LedModule_Init(&app->slave1_led, &led_config) == INFRA_STATUS_OK))
    {
        app->led1_enabled = 1U;
        app->slave1_mode = APP_SLAVE1_MODE_NORMAL;
    }

    return INFRA_STATUS_OK;
}

/*
 * CAN 제어 명령을 로컬 slave1 동작으로 바꾼다.
 * master는 이 경로를 통해,
 * slave1을 emergency 상태로 두거나 승인 완료 표시, 출력 해제를 지시한다.
 */
void AppSlave1_HandleCanCommand(AppCore *app,
                                const CanMessage *message,
                                uint8_t *out_response_code)
{
    char buffer[48];

    if ((app == NULL) || (message == NULL) || (out_response_code == NULL) || (app->led1_enabled == 0U))
    {
        return;
    }

    switch (message->payload[0])
    {
        case CAN_CMD_EMERGENCY:
            app->slave1_mode = APP_SLAVE1_MODE_EMERGENCY;
            LedModule_SetPattern(&app->slave1_led, LED_PATTERN_RED_SOLID);
            AppCore_SetModeText(app, "emergency");
            AppCore_SetButtonText(app, "press ok");
            AppCore_SetAdcText(app, "n/a");
            (void)snprintf(buffer, sizeof(buffer), "emergency in (%u)", (unsigned int)message->source_node_id);
            AppCore_SetCanInputText(app, buffer);
            *out_response_code = CAN_RES_OK;
            break;

        case CAN_CMD_OK:
            app->slave1_mode = APP_SLAVE1_MODE_ACK_BLINK;
            LedModule_StartGreenAckBlink(&app->slave1_led, APP_SLAVE1_ACK_TOGGLES);
            AppCore_SetModeText(app, "ack blink");
            AppCore_SetButtonText(app, "approved");
            AppCore_SetAdcText(app, "n/a");
            (void)snprintf(buffer, sizeof(buffer), "ok in (%u)", (unsigned int)message->source_node_id);
            AppCore_SetCanInputText(app, buffer);
            *out_response_code = CAN_RES_OK;
            break;

        case CAN_CMD_OFF:
            app->slave1_mode = APP_SLAVE1_MODE_NORMAL;
            LedModule_SetPattern(&app->slave1_led, LED_PATTERN_OFF);
            AppCore_SetModeText(app, "normal");
            AppCore_SetButtonText(app, "waiting");
            AppCore_SetAdcText(app, "n/a");
            AppCore_SetCanInputText(app, "waiting");
            *out_response_code = CAN_RES_OK;
            break;

        case CAN_CMD_OPEN:
        case CAN_CMD_CLOSE:
        case CAN_CMD_TEST:
            (void)snprintf(buffer,
                           sizeof(buffer),
                           "cmd %u in (%u)",
                           (unsigned int)message->payload[0],
                           (unsigned int)message->source_node_id);
            AppCore_SetCanInputText(app, buffer);
            *out_response_code = CAN_RES_OK;
            break;

        default:
            break;
    }
}

/*
 * 로컬 승인 버튼을 debounce하고 안정된 입력만 보고한다.
 * slave1은 master가 만든 emergency 상태에서 버튼이 눌릴 때만,
 * OK 요청을 올린다.
 */
void AppSlave1_TaskButton(AppCore *app, uint32_t now_ms)
{
    uint8_t raw_pressed;

    (void)now_ms;

    if ((app == NULL) || (app->can_enabled == 0U))
    {
        return;
    }

    raw_pressed = RuntimeIo_ReadSlave1ButtonPressed();
    if (raw_pressed == app->slave1_last_sample_pressed)
    {
        if (app->slave1_same_sample_count < 3U)
        {
            app->slave1_same_sample_count++;
        }
    }
    else
    {
        app->slave1_same_sample_count = 0U;
        app->slave1_last_sample_pressed = raw_pressed;
        return;
    }

    if (app->slave1_same_sample_count < 2U)
    {
        return;
    }

    if (app->slave1_stable_pressed == raw_pressed)
    {
        return;
    }

    app->slave1_stable_pressed = raw_pressed;
    if ((app->slave1_stable_pressed == 0U) || (app->slave1_mode != APP_SLAVE1_MODE_EMERGENCY))
    {
        return;
    }

    AppCore_SetButtonText(app, "pressed");
    if (AppCore_QueueCanCommandCode(app, APP_NODE_ID_MASTER, CAN_CMD_OK, 0U) != 0U)
    {
        AppCore_SetResultText(app, "[can] ok request sent");
    }
}

/*
 * 로컬 LED pattern 상태기계를 진행시킨다.
 * 유한한 acknowledgement blink 시퀀스를 여기서 끝까지 수행하여,
 * slave1이 이후 정상 standby 상태로 돌아가게 한다.
 */
void AppSlave1_TaskLed(AppCore *app, uint32_t now_ms)
{
    if ((app == NULL) || (app->led1_enabled == 0U))
    {
        return;
    }

    LedModule_Task(&app->slave1_led, now_ms);
    if ((app->slave1_mode == APP_SLAVE1_MODE_ACK_BLINK) &&
        (LedModule_GetPattern(&app->slave1_led) == LED_PATTERN_OFF))
    {
        app->slave1_mode = APP_SLAVE1_MODE_NORMAL;
        AppCore_SetModeText(app, "normal");
        AppCore_SetButtonText(app, "waiting");
    }
}
