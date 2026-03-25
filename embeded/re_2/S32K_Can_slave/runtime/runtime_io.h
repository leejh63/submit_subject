/*
 * portable 모듈과 보드를 연결하는 하드웨어 바인딩 인터페이스다.
 * 상위 계층은 SDK 세부 구현을 직접 건드리지 않고,
 * 이 헤더를 통해 role, ADC, LIN, LED, 버튼 자원을 요청한다.
 */
#ifndef RUNTIME_IO_H
#define RUNTIME_IO_H

#include "adc/adc_module.h"
#include "app/app_config.h"
#include "infra/infra_types.h"
#include "led/led_module.h"
#include "lin/lin_module.h"

/*
 * portable 모듈이 사용하는 역할/하드웨어 바인딩 조회 API다.
 * runtime과 app 계층은 SDK symbol에 직접 의존하지 않고,
 * 여기서 LED, ADC, LIN, 버튼 자원을 요청한다.
 */
InfraStatus RuntimeIo_BoardInit(void);
uint8_t     RuntimeIo_GetActiveRole(void);
uint8_t     RuntimeIo_GetLocalNodeId(void);

InfraStatus RuntimeIo_GetSlave1LedConfig(LedConfig *out_config);
InfraStatus RuntimeIo_GetSlave2LedConfig(LedConfig *out_config);
uint8_t     RuntimeIo_ReadSlave1ButtonPressed(void);

InfraStatus RuntimeIo_GetSlave2AdcConfig(AdcConfig *out_config);
InfraStatus RuntimeIo_GetMasterLinConfig(LinConfig *out_config);
InfraStatus RuntimeIo_GetSlaveLinConfig(LinConfig *out_config);
void        RuntimeIo_AttachLinModule(LinModule *module);
void        RuntimeIo_LinNotifyEvent(LinEventId event_id, uint8_t current_pid);

#endif
