#include "app_uart_console_task.h"

#include <stddef.h>
#include <string.h>

void AppUartConsoleTask_Clear(AppUartConsoleTaskContext *task)
{
    if (task == NULL)
        return;

    (void)memset(task, 0, sizeof(*task));
}

void AppUartConsoleTask_Init(AppUartConsoleTaskContext *task, AppUartConsoleContext *node)
{
    if (task == NULL)
        return;

    AppUartConsoleTask_Clear(task);
    task->node = node;
}

void AppUartConsoleTask_Run(AppUartConsoleTaskContext *task, uint32_t nowMs)
{
    AppUartConsoleContext *node;

    if (task == NULL)
        return;

    node = task->node;
    if (node == NULL)
        return;

    AppUartConsole_RunRxStage(node);

    if (AppUartConsole_IsErrorState(node) == 0U)
    {
        AppUartConsole_RunInputStage(node);
        AppUartConsole_RunResultStage(node);
    }
    else
    {
        AppUartConsole_RunRecoverStage(node);
    }

    AppUartConsole_RunTxStage(node, nowMs);
}

void AppUartConsoleTask_Render(AppUartConsoleTaskContext *task)
{
    if (task == NULL || task->node == NULL)
        return;

    AppUartConsole_RenderTask(task->node);
}
