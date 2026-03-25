#include "hal/hal_s32k_gpio.h"
#include "sdk_project_config.h"

EmbResult HalS32kGpio_SetDirection(void *gpioBase, uint32_t pinMask)
{
    if (gpioBase == 0)
        return EMB_EINVAL;

    PINS_DRV_SetPinsDirection((GPIO_Type *)gpioBase, pinMask);
    return EMB_OK;
}

EmbResult HalS32kGpio_WritePin(void *gpioBase, uint32_t pin, uint8_t value)
{
    if (gpioBase == 0)
        return EMB_EINVAL;

    PINS_DRV_WritePin((GPIO_Type *)gpioBase, pin, (uint8_t)((value != 0U) ? 1U : 0U));
    return EMB_OK;
}

EmbResult HalS32kGpio_SetPins(void *gpioBase, uint32_t pinMask)
{
    if (gpioBase == 0)
        return EMB_EINVAL;

    PINS_DRV_SetPins((GPIO_Type *)gpioBase, pinMask);
    return EMB_OK;
}
