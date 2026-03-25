/*
 * master 노드가 실제로 사용하는 보드 바인딩 인터페이스다.
 * coordinator는 로컬 node id와 LIN master 설정만 필요하므로,
 * slave용 LED, 버튼, ADC API는 제거해 인터페이스를 단순화한다.
 */
#ifndef RUNTIME_IO_H
#define RUNTIME_IO_H

#include "infra/infra_types.h"
#include "lin/lin_module.h"

InfraStatus RuntimeIo_BoardInit(void);
uint8_t     RuntimeIo_GetLocalNodeId(void);
InfraStatus RuntimeIo_GetMasterLinConfig(LinConfig *out_config);
void        RuntimeIo_AttachLinModule(LinModule *module);
void        RuntimeIo_LinNotifyEvent(LinEventId event_id, uint8_t current_pid);

#endif
