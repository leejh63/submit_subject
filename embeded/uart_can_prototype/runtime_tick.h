#ifndef RUNTIME_TICK_H
#define RUNTIME_TICK_H

#include <stdint.h>

#include "status.h"

status_t RuntimeTick_Init(void);
uint32_t RuntimeTick_GetMs(void);

#endif
