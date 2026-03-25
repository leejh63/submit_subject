#include "isosdk_board.h"

#include <stddef.h>

#include "pins_driver.h"

#include "isosdk_board_profile.h"
#include "isosdk_sdk_bindings.h"

uint8_t IsoSdk_BoardInit(void)
{
    status_t status;

    status = CLOCK_SYS_Init(ISOSDK_SDK_CLOCK_CONFIGS,
                            ISOSDK_SDK_CLOCK_CONFIG_COUNT,
                            ISOSDK_SDK_CLOCK_CALLBACKS,
                            ISOSDK_SDK_CLOCK_CALLBACK_COUNT);
    if (status != STATUS_SUCCESS)
    {
        return 0U;
    }

    status = CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
    if (status != STATUS_SUCCESS)
    {
        return 0U;
    }

    PINS_DRV_Init(ISOSDK_SDK_PIN_CONFIG_COUNT, ISOSDK_SDK_PIN_CONFIGS);
    return 1U;
}

void IsoSdk_BoardEnableLinTransceiver(void)
{
#ifdef ISOSDK_SDK_HAS_LIN
    PINS_DRV_SetPinsDirection(ISOSDK_BOARD_PROFILE_LIN_XCVR_ENABLE_PORT,
                              ISOSDK_BOARD_PROFILE_LIN_XCVR_ENABLE_MASK);
    PINS_DRV_SetPins(ISOSDK_BOARD_PROFILE_LIN_XCVR_ENABLE_PORT,
                     ISOSDK_BOARD_PROFILE_LIN_XCVR_ENABLE_MASK);
#endif
}

void *IsoSdk_BoardGetRgbLedPort(void)
{
    return ISOSDK_BOARD_PROFILE_RGB_LED_PORT;
}

uint32_t IsoSdk_BoardGetRgbLedRedPin(void)
{
    return ISOSDK_BOARD_PROFILE_RGB_LED_RED_PIN;
}

uint32_t IsoSdk_BoardGetRgbLedGreenPin(void)
{
    return ISOSDK_BOARD_PROFILE_RGB_LED_GREEN_PIN;
}

uint8_t IsoSdk_BoardGetRgbLedActiveOnLevel(void)
{
    return ISOSDK_BOARD_PROFILE_RGB_LED_ACTIVE_ON_LEVEL;
}

uint8_t IsoSdk_BoardReadSlave1ButtonPressed(void)
{
    GPIO_Type *gpio_port;

    gpio_port = (GPIO_Type *)ISOSDK_BOARD_PROFILE_SLAVE1_BUTTON_PORT;
    return ((PINS_DRV_ReadPins(gpio_port) & ISOSDK_BOARD_PROFILE_SLAVE1_BUTTON_MASK) == 0U) ? 1U : 0U;
}

void IsoSdk_GpioWritePin(void *gpio_port, uint32_t pin, uint8_t level)
{
    if (gpio_port == NULL)
    {
        return;
    }

    PINS_DRV_WritePin((GPIO_Type *)gpio_port, pin, level);
}

void IsoSdk_GpioSetPinsDirectionMask(void *gpio_port, uint32_t pin_mask)
{
    if (gpio_port == NULL)
    {
        return;
    }

    PINS_DRV_SetPinsDirection((GPIO_Type *)gpio_port,
                              (pins_channel_type_t)pin_mask);
}
