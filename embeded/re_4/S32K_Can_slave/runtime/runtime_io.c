/*
 * CAN 현장 반응 slave용 실제 보드 바인딩 구현부다.
 * slave1에 필요한 버튼 입력과 LED 배선만 남기고,
 * 플랫폼 세부사항은 driver 계층 아래로 숨긴다.
 */
#include "runtime_io.h"

#include "../drivers/board_hw.h"
#include "../app/app_config.h"

InfraStatus RuntimeIo_BoardInit(void)
{
    return BoardHw_Init();
}

uint8_t RuntimeIo_GetLocalNodeId(void)
{
    return APP_NODE_ID_SLAVE1;
}

InfraStatus RuntimeIo_GetSlave1LedConfig(LedConfig *out_config)
{
    return BoardHw_GetRgbLedConfig(out_config);
}

uint8_t RuntimeIo_ReadSlave1ButtonPressed(void)
{
    return BoardHw_ReadSlave1ButtonPressed();
}
