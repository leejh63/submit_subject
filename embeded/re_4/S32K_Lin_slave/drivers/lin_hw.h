#ifndef LIN_HW_H
#define LIN_HW_H

#include "../core/infra_types.h"
#include "../services/lin_module.h"

uint8_t     LinHw_IsSupported(void);
void        LinHw_Configure(uint8_t role, uint16_t timeout_ticks);
void        LinHw_AttachModule(LinModule *module);
InfraStatus LinHw_Init(void *context);
InfraStatus LinHw_MasterSendHeader(void *context, uint8_t pid);
InfraStatus LinHw_StartReceive(void *context, uint8_t *buffer, uint8_t length);
InfraStatus LinHw_StartSend(void *context, const uint8_t *buffer, uint8_t length);
void        LinHw_GotoIdle(void *context);
void        LinHw_SetTimeout(void *context, uint16_t timeout_ticks);
void        LinHw_ServiceTick(void *context);

#endif
