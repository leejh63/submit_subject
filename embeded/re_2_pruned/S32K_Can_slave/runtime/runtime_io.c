/*
 * CAN 현장 반응 slave용 실제 보드 바인딩 구현부다.
 * slave1에 필요한 버튼 입력과 LED 배선만 남겨,
 * SDK 세부사항은 IsoSdk 계층으로 격리한다.
 */
#include "runtime_io.h"

#include <stddef.h>
#include <string.h>

#include "../../isosdk/isosdk_board.h"
#include "app/app_config.h"

InfraStatus RuntimeIo_BoardInit(void)
{
    return (IsoSdk_BoardInit() != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

uint8_t RuntimeIo_GetLocalNodeId(void)
{
    return APP_NODE_ID_SLAVE1;
}

InfraStatus RuntimeIo_GetSlave1LedConfig(LedConfig *out_config)
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

uint8_t RuntimeIo_ReadSlave1ButtonPressed(void)
{
    return IsoSdk_BoardReadSlave1ButtonPressed();
}
