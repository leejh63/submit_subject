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

    // ============================================================
    // 시스템 초기화 임시
    // ============================================================

    CLOCK_SYS_Init(g_clockManConfigsArr,
                   CLOCK_MANAGER_CONFIG_CNT,
                   g_clockManCallbacksArr,
                   CLOCK_MANAGER_CALLBACK_CNT);

    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    // ============================================================
    // 런타임 초기화
    // ============================================================

    status = Runtime_Init();
    if (status != STATUS_SUCCESS)
    {
        Runtime_FaultLoop();
    }

    // ============================================================
    // 메인 루프
    // ============================================================

    while (1)
    {
        Runtime_Run();

        if (exit_code != 0) { break; }
    }

    return exit_code;
}
