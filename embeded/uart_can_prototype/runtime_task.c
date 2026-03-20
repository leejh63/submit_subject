#include "runtime_task.h"

void RuntimeTask_RunTable(RuntimeTaskEntry *table,
                          uint32_t taskCount,
                          uint32_t currentTickMs)
{
    uint32_t i;

    if (table == (void *)0)
    {
        return;
    }

    for (i = 0U; i < taskCount; i++)
    {
        RuntimeTaskEntry *task = &table[i];

        if (task->taskFunc == (void *)0)
        {
            continue;
        }

        if (task->periodMs == 0U)
        {
            continue;
        }

        if ((currentTickMs - task->lastRunTick) >= task->periodMs)
        {
            task->lastRunTick = currentTickMs;
            task->taskFunc(task->context);
        }
    }
}
