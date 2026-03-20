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
// 명확하게 rx, tx경계가 분할안되어있는상황인듯?
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
// 여기서 rx/tx 을 진행
// rx는 hw에서 부터 차례대로 올라오면서 4가지 길로 갈라짐
// 각각 큐도 존재 중간에 올라오면서 디코딩진행 이 디코딩 과정이 상당히 별로임
// 태스크 타고 내려가다보면 CanTransport_ProcessTx에서 
// tx를 한번 진행함
// 
    CanApp_RunService(app, nowMs);
// rx큐를 종류별로 pop해서 각각 알맞는 방식으로 처리진행
    CanApp_RunRemoteCommands(app);
    CanApp_RunServiceResults(app);
    CanApp_RunEvents(app);
    CanApp_RunTexts(app);

// 여기서 입력에 따른 인코딩 진행 후 tx 큐삽입 진행
    if (task->commandMailbox != NULL)
    {
        for (handled = 0U; handled < task->maxSubmitPerTick; handled++)
        {
            popped = AppCtrlCommandMailbox_Pop(task->commandMailbox, &cmd);
            if (popped == 0U)
                break;

            activity = 1U;
            // 반환값 사용안함 // 내부 분기 처리가 조금 맘에 안듬
            (void)CanApp_SubmitAppCtrlCommand(app, &cmd);
        }
    }
//  사실상 CanTransport_Task함수 호출용
    CanApp_FlushTx(app, nowMs);

// 위에서 can rx 큐에 담은 결과값들을 uart쪽 tx 큐에 삽입한다.
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
