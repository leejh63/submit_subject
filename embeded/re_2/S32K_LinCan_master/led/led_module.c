/*
 * 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다.
 * 의미 기반 패턴을 GPIO write로 바꾸고,
 * 주기 LED task 동안 blink 시퀀스를 진행시킨다.
 */
#include "led_module.h"

#include <stddef.h>
#include <string.h>

#include "pins_driver.h"

/*
 * 보드의 active level 규칙에 맞춰 LED pin 하나를 쓴다.
 * 이 변환을 한곳에 모아두면,
 * 나머지 모듈 코드가 polarity 전용 GPIO 로직에서 자유로워진다.
 */
static void LedModule_WritePin(const LedModule *module, uint32_t pin, uint8_t on)
{
    GPIO_Type *gpio_port;
    uint8_t    level;

    if ((module == NULL) || (module->config.gpio_port == NULL))
    {
        return;
    }

    gpio_port = (GPIO_Type *)module->config.gpio_port;
    level = (on != 0U) ? module->config.active_on_level :
            (uint8_t)((module->config.active_on_level == 0U) ? 1U : 0U);

    PINS_DRV_WritePin(gpio_port, pin, level);
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

/*
 * 의미 기반 LED pattern을 실제 pin 출력으로 변환한다.
 * blink pattern은 현재 phase flag를 사용하고,
 * solid 상태는 요청된 색 조합으로 바로 매핑된다.
 */
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

/*
 * 로컬 보드 pin에 대한 LED 모듈 인스턴스를 초기화한다.
 * GPIO 방향 설정을 여기서 끝내 두면,
 * 이후 pattern 갱신은 캐시된 의미 상태만 바꾸면 된다.
 */
InfraStatus LedModule_Init(LedModule *module, const LedConfig *config)
{
    GPIO_Type *gpio_port;

    if ((module == NULL) || (config == NULL) || (config->gpio_port == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(module, 0, sizeof(*module));
    module->config = *config;
    module->pattern = LED_PATTERN_OFF;
    module->output_phase_on = 0U;

    gpio_port = (GPIO_Type *)module->config.gpio_port;
    PINS_DRV_SetPinsDirection(gpio_port,
                              (pins_channel_type_t)((1UL << module->config.red_pin) |
                                                    (1UL << module->config.green_pin)));

    LedModule_ApplyPattern(module, module->pattern, 0U);
    module->initialized = 1U;

    return INFRA_STATUS_OK;
}

/*
 * 새로운 steady 또는 반복 pattern으로 즉시 전환한다.
 * caller가 명시적으로 새 LED 상태를 요구한 것이므로,
 * 이전 finite blink 시퀀스는 취소된다.
 */
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

/*
 * finite green acknowledgement blink 시퀀스를 시작한다.
 * slave1은 이 pattern을 사용해 승인 완료를 보여주고,
 * 이후 자동으로 idle LED 상태로 돌아간다.
 */
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

/*
 * 주기 LED task 동안 blink pattern을 한 단계 진행한다.
 * solid pattern은 바로 반환하고,
 * finite blink 모드는 남은 toggle 수를 줄이며 끝나면 스스로 꺼진다.
 */
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
