#include "board_hw.h"

#include <stddef.h>
#include <string.h>

#include "../platform/s32k_sdk/isosdk_board.h"

InfraStatus BoardHw_Init(void)
{
    return (IsoSdk_BoardInit() != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

void BoardHw_EnableLinTransceiver(void)
{
    IsoSdk_BoardEnableLinTransceiver();
}

InfraStatus BoardHw_GetRgbLedConfig(LedConfig *out_config)
{
    if (out_config == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(out_config, 0, sizeof(*out_config));
    out_config->gpio_port = IsoSdk_BoardGetRgbLedPort();
    out_config->red_pin = IsoSdk_BoardGetRgbLedRedPin();
    out_config->green_pin = IsoSdk_BoardGetRgbLedGreenPin();
    out_config->active_on_level = IsoSdk_BoardGetRgbLedActiveOnLevel();
    return INFRA_STATUS_OK;
}

uint8_t BoardHw_ReadSlave1ButtonPressed(void)
{
    return IsoSdk_BoardReadSlave1ButtonPressed();
}
