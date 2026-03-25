#include "drivers/drv_led.h"
#include "hal/hal_s32k_gpio.h"

EmbResult DrvLed_Init(DrvLed *led, const DrvLedConfig *config)
{
    if (led == 0 || config == 0)
        return EMB_EINVAL;

    led->config = *config;
    led->started = 0U;
    return EMB_OK;
}

EmbResult DrvLed_Start(DrvLed *led)
{
    if (led == 0)
        return EMB_EINVAL;

    (void)HalS32kGpio_SetDirection(led->config.gpioBase, led->config.pinMask);
    (void)HalS32kGpio_WritePin(led->config.gpioBase,
                               led->config.redPin,
                               led->config.inactiveLevel);
    (void)HalS32kGpio_WritePin(led->config.gpioBase,
                               led->config.greenPin,
                               led->config.inactiveLevel);
    led->started = 1U;
    return EMB_OK;
}

EmbResult DrvLed_SetRedGreen(DrvLed *led, uint8_t redOn, uint8_t greenOn)
{
    if (led == 0 || led->started == 0U)
        return EMB_ESTATE;

    (void)HalS32kGpio_WritePin(led->config.gpioBase,
                               led->config.redPin,
                               (redOn != 0U) ? led->config.activeLevel : led->config.inactiveLevel);
    (void)HalS32kGpio_WritePin(led->config.gpioBase,
                               led->config.greenPin,
                               (greenOn != 0U) ? led->config.activeLevel : led->config.inactiveLevel);
    return EMB_OK;
}
