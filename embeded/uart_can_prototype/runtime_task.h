#ifndef RUNTIME_TASK_H
#define RUNTIME_TASK_H

#include <stdint.h>

typedef void (*RuntimeTaskFunc)(void *context);

typedef struct
{
    const char      *name;
    uint32_t         periodMs;
    uint32_t         lastRunTick;
    RuntimeTaskFunc  taskFunc;
    void            *context;
} RuntimeTaskEntry;

void RuntimeTask_RunTable(RuntimeTaskEntry *table,
                          uint32_t taskCount,
                          uint32_t currentTickMs);

#endif
