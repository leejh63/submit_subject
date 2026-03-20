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
        // 현재 라인을 완성후에 텍스트 결과를 저장해 놨지만
        // 만약 can 수신을 받아 rx큐에 데이터가 있다면
        // can 결과로 텍스트 결과를 덮어 씀
        AppUartConsole_RunResultStage(node);
    }
    else
    {
        AppUartConsole_RunRecoverStage(node);
    }
// 기존에는 result 큐에 데이터가 존재하면 진행
// 즉 uart송신의 경우 1ms 마다 실행을 진행하지만
// 예외나 특정상황 제외 현재는 
// 큐에 집어 넣는 과정은 100ms 태스크 랜더에서 진행함
    AppUartConsole_RunTxStage(node, nowMs);
}

void AppUartConsoleTask_Render(AppUartConsoleTaskContext *task)
{
    if (task == NULL || task->node == NULL)
        return;

    AppUartConsole_RenderTask(task->node);
}
