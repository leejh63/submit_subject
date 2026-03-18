#include "runtime_tick.h"

#include <stddef.h>
#include "sdk_project_config.h"
#include "interrupt_manager.h"

static volatile uint32_t g_runtimeTickMs = 0U;

static void RuntimeTick_IrqHandler(void);

status_t RuntimeTick_Init(void)
{
    g_runtimeTickMs = 0U;

    LPTMR_DRV_Init(INST_LPTMR_1, &lptmr_1_config0, false);
    INT_SYS_InstallHandler(LPTMR0_IRQn, RuntimeTick_IrqHandler, (isr_t *)NULL);
    INT_SYS_EnableIRQ(LPTMR0_IRQn);
    LPTMR_DRV_StartCounter(INST_LPTMR_1);

    return STATUS_SUCCESS;
}

uint32_t RuntimeTick_GetMs(void)
{
    return g_runtimeTickMs;
}

static void RuntimeTick_IrqHandler(void)
{
    g_runtimeTickMs++;
    LPTMR_DRV_ClearCompareFlag(INST_LPTMR_1);
}
