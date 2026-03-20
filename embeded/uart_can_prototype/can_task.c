#include "can_task.h"

#include <stddef.h>
#include <string.h>

void CanTask_Clear(CanTaskContext *task)
{
    if (task == NULL)
        return;

    (void)memset(task, 0, sizeof(*task));
    task->maxSubmitPerTick = CAN_TASK_DEFAULT_MAX_SUBMIT_PER_TICK;
    task->maxResultPerTick = CAN_TASK_DEFAULT_MAX_RESULT_PER_TICK;
}

void CanTask_Init(CanTaskContext *task,
                  CanApp *app,
                  AppCtrlCommandMailbox *commandMailbox,
                  AppCtrlResultBox *resultBox)
{
    if (task == NULL)
        return;

    CanTask_Clear(task);
    task->app = app;
    task->commandMailbox = commandMailbox;
    task->resultBox = resultBox;
}

uint8_t CanTask_Run(CanTaskContext *task, uint32_t nowMs)
{
    CanApp     *app;
    AppCtrlCommand     cmd;
    AppCtrlResult  result;
    uint8_t     handled;
    uint8_t     popped;
    uint8_t     activity;

    if (task == NULL || task->app == NULL)
        return 0U;

    app = task->app;
    activity = 0U;

    CanApp_RunService(app, nowMs);
    CanApp_RunRemoteCommands(app);
    CanApp_RunServiceResults(app);
    CanApp_RunEvents(app);
    CanApp_RunTexts(app);

    if (task->commandMailbox != NULL)
    {
        for (handled = 0U; handled < task->maxSubmitPerTick; handled++)
        {
            popped = AppCtrlCommandMailbox_Pop(task->commandMailbox, &cmd);
            if (popped == 0U)
                break;

            activity = 1U;
            (void)CanApp_SubmitAppCtrlCommand(app, &cmd);
        }
    }

    CanApp_FlushTx(app, nowMs);

    if (task->resultBox != NULL)
    {
        for (handled = 0U; handled < task->maxResultPerTick; handled++)
        {
            popped = CanApp_PopAppCtrlResult(app, &result);
            if (popped == 0U)
                break;

            activity = 1U;
            (void)AppCtrlResultBox_Push(task->resultBox, &result);
        }
    }

    task->lastActivity = activity;
    return activity;
}
