#ifndef S32K_TICK_H
#define S32K_TICK_H

#include <stdint.h>
#include "core/emb_time.h"
#include "core/emb_critical.h"

void S32kTick_Init(void);
emb_tick_t S32kTick_Get500us(void);
uint32_t S32kTick_GetMs(void);
void S32kTick_IrqHandler(void);
uint32_t S32kLinTimeoutTimeNsCallback(uint32_t *ns);

#endif
