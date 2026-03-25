/*
 * slave2 policy 구현부다.
 * ADC 값을 샘플링하고 최신 LIN 상태 프레임을 게시하며,
 * 해석된 센서 상태에 따라 로컬 LED 피드백도 갱신한다.
 */
#include "app_slave2.h"

#include <stddef.h>
#include <stdio.h>

#include "app_core_internal.h"
#include "../runtime/runtime_io.h"

/*
 * 센서 노드 역할을 초기화한다.
 * slave2는 로컬 LED 피드백과 ADC 샘플링,
 * LIN slave 상태 제공을 모두 맡기 때문에 세 모듈을 여기서 함께 시작한다.
 */
InfraStatus AppSlave2_Init(AppCore *app)
{
    LedConfig led_config;
    AdcConfig adc_config;
    LinConfig lin_config;

    if (app == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if ((RuntimeIo_GetSlave2LedConfig(&led_config) == INFRA_STATUS_OK) &&
        (LedModule_Init(&app->slave2_led, &led_config) == INFRA_STATUS_OK))
    {
        app->led2_enabled = 1U;
    }

    if ((RuntimeIo_GetSlave2AdcConfig(&adc_config) == INFRA_STATUS_OK) &&
        (AdcModule_Init(&app->adc_module, &adc_config) == INFRA_STATUS_OK))
    {
        app->adc_enabled = 1U;
    }
    else
    {
        AppCore_SetAdcText(app, "binding req");
    }

    if ((RuntimeIo_GetSlaveLinConfig(&lin_config) == INFRA_STATUS_OK) &&
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
 * LIN으로 전달된 승인 token을 소비한다.
 * 이 token은 latch를 강제로 지우지 않고,
 * zone이 더 이상 emergency가 아닐 때만 ADC 모듈에 clear를 요청한다.
 */
void AppSlave2_HandleLinOkToken(AppCore *app)
{
    if ((app == NULL) || (app->adc_enabled == 0U))
    {
        return;
    }

    if ((LinModule_ConsumeSlaveOkToken(&app->lin_module) != 0U) &&
        (AdcModule_ClearEmergencyLatch(&app->adc_module) == INFRA_STATUS_OK))
    {
        AppCore_SetLinInputText(app, "ok token in");
    }
}

/*
 * ADC 값을 샘플링하고 최신 해석 상태를 게시한다.
 * 이 task는 센서 획득과 LIN 상태 cache 갱신,
 * 콘솔 문자열과 로컬 LED 피드백을 함께 묶는 지점이다.
 */
void AppSlave2_TaskAdc(AppCore *app, uint32_t now_ms)
{
    AdcSnapshot    snapshot;
    LinStatusFrame status;

    if ((app == NULL) || (app->adc_enabled == 0U))
    {
        return;
    }

    AdcModule_Task(&app->adc_module, now_ms);
    if (AdcModule_GetSnapshot(&app->adc_module, &snapshot) != INFRA_STATUS_OK)
    {
        return;
    }

    (void)snprintf(app->adc_text,
                   sizeof(app->adc_text),
                   "%u (%s, lock=%u)",
                   (unsigned int)snapshot.raw_value,
                   AdcModule_ZoneText(snapshot.zone),
                   (unsigned int)snapshot.emergency_latched);

    status.adc_value = snapshot.raw_value;
    status.zone = snapshot.zone;
    status.emergency_latched = snapshot.emergency_latched;
    status.valid = 1U;
    status.fresh = 1U;
    if (app->lin_enabled != 0U)
    {
        LinModule_SetSlaveStatus(&app->lin_module, &status);
    }

    if (app->led2_enabled != 0U)
    {
        if (snapshot.emergency_latched != 0U)
        {
            LedModule_SetPattern(&app->slave2_led, LED_PATTERN_RED_BLINK);
        }
        else if (snapshot.zone == ADC_ZONE_SAFE)
        {
            LedModule_SetPattern(&app->slave2_led, LED_PATTERN_GREEN_SOLID);
        }
        else if (snapshot.zone == ADC_ZONE_WARNING)
        {
            LedModule_SetPattern(&app->slave2_led, LED_PATTERN_YELLOW_SOLID);
        }
        else
        {
            LedModule_SetPattern(&app->slave2_led, LED_PATTERN_RED_SOLID);
        }
    }
}
