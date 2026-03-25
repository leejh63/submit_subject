/*
 * CAN 현장 반응 slave가 실제로 사용하는 보드 바인딩 인터페이스다.
 * slave1은 버튼과 로컬 LED 배선만 필요하므로,
 * LIN과 ADC 관련 API를 제거해 보드 계층을 단순화한다.
 */
#ifndef RUNTIME_IO_H
#define RUNTIME_IO_H

#include "infra/infra_types.h"
#include "led/led_module.h"

InfraStatus RuntimeIo_BoardInit(void);
uint8_t     RuntimeIo_GetLocalNodeId(void);
InfraStatus RuntimeIo_GetSlave1LedConfig(LedConfig *out_config);
uint8_t     RuntimeIo_ReadSlave1ButtonPressed(void);

#endif
