#ifndef CAN_TASK_H
#define CAN_TASK_H

#include <stdint.h>

#include "can_app.h"
#include "app_ctrl_command_mailbox.h"
#include "app_ctrl_result_box.h"

#define CAN_TASK_DEFAULT_MAX_SUBMIT_PER_TICK   2U
#define CAN_TASK_DEFAULT_MAX_RESULT_PER_TICK   4U

// CanApp 위에 얹는 얇은 orchestration 레이어.
// 현재 기본 흐름은 유지하되, 이후에는 CanApp step API를 직접 조합해서
// mailbox/result 연결 정책이나 frame/message 정책을 더 자유롭게 바꾸기 쉽게 만든다.

typedef struct
{
    CanApp        *app;
    AppCtrlCommandMailbox   *commandMailbox;
    AppCtrlResultBox *resultBox;

    uint8_t        maxSubmitPerTick;
    uint8_t        maxResultPerTick;
    uint8_t        lastActivity;
} CanTaskContext;

void    CanTask_Clear(CanTaskContext *task);
void    CanTask_Init(CanTaskContext *task,
                     CanApp *app,
                     AppCtrlCommandMailbox *commandMailbox,
                     AppCtrlResultBox *resultBox);
uint8_t CanTask_Run(CanTaskContext *task, uint32_t nowMs);

#endif
