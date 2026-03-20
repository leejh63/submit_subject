/*!
** Copyright 2020 NXP
** @file main.c
** @brief
**         Main module.
*/

#include "sdk_project_config.h"
#include "runtime.h"

volatile int exit_code = 0;

int main(void)
{
    status_t status;

    CLOCK_SYS_Init(g_clockManConfigsArr,
                   CLOCK_MANAGER_CONFIG_CNT,
                   g_clockManCallbacksArr,
                   CLOCK_MANAGER_CALLBACK_CNT);

    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
// 전체적으로 함수명 변수명 그리고 변수접근방식 캡슐화등 맘에 안듬 
// 레이어는 그럭저럭 나눈것 같지만 맘에 안드는 부분이 상당함
// 현재 태스크 처리 또한 그저 풀링 방식이지만 이걸 이벤트 형식이 좀더 적절해보임
// 또한 각각 태스크 내부에서 풀링 방식으로 큐를 비워버리는 일을 하는데
// 이거에 대한 대처도 필요해보임
// 일단 태스크 처리 방식의 경우 지금당장 수정사항은 아닌듯 보임
// 추후 수정 고민중
    status = Runtime_Init();
    if (status != STATUS_SUCCESS)
    {
        Runtime_FaultLoop();
    }

    while (1)
    {
        Runtime_Run();

        if (exit_code != 0) { break; }
    }

    return exit_code;
}
