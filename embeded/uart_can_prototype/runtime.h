#ifndef RUNTIME_H
#define RUNTIME_H

#include "status.h"

status_t Runtime_Init(void);
void Runtime_Run(void);
void Runtime_FaultLoop(void);

#endif
