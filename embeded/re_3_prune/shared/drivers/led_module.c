/*
 * 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다.
 * 의미 기반 패턴을 GPIO write로 바꾸고,
 * 주기 LED task 동안 blink 시퀀스를 진행시킨다.
 */
#include "led_module.h"

#include <stddef.h>
#include <string.h>

#include "../../platform/s32k_sdk/isosdk_board.h"

static void LedModule_WritePin(const LedModule *module, uint32_t pin, uint8_t on)
{
    uint8_t level;

    if ((module == NULL) || (module->config.gpio_port == NULL))
    {
        return;
    }

    level = (on != 0U) ? module->config.active_on_level :
            (uint8_t)((module->config.active_on_level == 0U) ? 1U : 0U);

    IsoSdk_GpioWritePin(module->config.gpio_port, pin, level);
}

static void LedModule_ApplyOutputs(const LedModule *module, uint8_t red_on, uint8_t green_on)
{
    if (module == NULL)
    {
        return;
    }

    LedModule_WritePin(module, module->config.red_pin, red_on);
    LedModule_WritePin(module, module->config.green_pin, green_on);
}

static void LedModule_ApplyPattern(const LedModule *module, LedPattern pattern, uint8_t phase_on)
{
    switch (pattern)
    {
        case LED_PATTERN_GREEN_SOLID:
            LedModule_ApplyOutputs(module, 0U, 1U);
            break;

        case LED_PATTERN_RED_SOLID:
            LedModule_ApplyOutputs(module, 1U, 0U);
            break;

        case LED_PATTERN_YELLOW_SOLID:
            LedModule_ApplyOutputs(module, 1U, 1U);
            break;

        case LED_PATTERN_RED_BLINK:
            LedModule_ApplyOutputs(module, phase_on, 0U);
            break;

        case LED_PATTERN_GREEN_BLINK:
            LedModule_ApplyOutputs(module, 0U, phase_on);
            break;

        case LED_PATTERN_OFF:
        default:
            LedModule_ApplyOutputs(module, 0U, 0U);
            break;
    }
}

InfraStatus LedModule_Init(LedModule *module, const LedConfig *config)
{
    if ((module == NULL) || (config == NULL) || (config->gpio_port == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(module, 0, sizeof(*module));
    module->config = *config;
    module->pattern = LED_PATTERN_OFF;
    module->output_phase_on = 0U;

    IsoSdk_GpioSetPinsDirectionMask(module->config.gpio_port,
                                    (1UL << module->config.red_pin) |
                                    (1UL << module->config.green_pin));

    LedModule_ApplyPattern(module, module->pattern, 0U);
    module->initialized = 1U;

    return INFRA_STATUS_OK;
}

void LedModule_SetPattern(LedModule *module, LedPattern pattern)
{
    if ((module == NULL) || (module->initialized == 0U))
    {
        return;
    }

    module->pattern = pattern;
    module->finite_blink_enabled = 0U;
    module->blink_toggles_remaining = 0U;
    module->output_phase_on = (pattern == LED_PATTERN_RED_BLINK ||
                               pattern == LED_PATTERN_GREEN_BLINK) ? 1U : 0U;

    LedModule_ApplyPattern(module, module->pattern, module->output_phase_on);
}

void LedModule_StartGreenAckBlink(LedModule *module, uint8_t toggle_count)
{
    if ((module == NULL) || (module->initialized == 0U))
    {
        return;
    }

    module->pattern = LED_PATTERN_GREEN_BLINK;
    module->output_phase_on = 1U;
    module->finite_blink_enabled = 1U;
    module->blink_toggles_remaining = toggle_count;

    LedModule_ApplyPattern(module, module->pattern, module->output_phase_on);
}

void LedModule_Task(LedModule *module, uint32_t now_ms)
{
    (void)now_ms;

    if ((module == NULL) || (module->initialized == 0U))
    {
        return;
    }

    if ((module->pattern != LED_PATTERN_RED_BLINK) &&
        (module->pattern != LED_PATTERN_GREEN_BLINK))
    {
        return;
    }

    module->output_phase_on = (module->output_phase_on == 0U) ? 1U : 0U;

    if (module->finite_blink_enabled != 0U)
    {
        if (module->blink_toggles_remaining > 0U)
        {
            module->blink_toggles_remaining--;
        }

        if (module->blink_toggles_remaining == 0U)
        {
            module->finite_blink_enabled = 0U;
            module->pattern = LED_PATTERN_OFF;
            module->output_phase_on = 0U;
            LedModule_ApplyPattern(module, module->pattern, module->output_phase_on);
            return;
        }
    }

    LedModule_ApplyPattern(module, module->pattern, module->output_phase_on);
}

LedPattern LedModule_GetPattern(const LedModule *module)
{
    if (module == NULL)
    {
        return LED_PATTERN_OFF;
    }

    return module->pattern;
}
