#include "platform/s32k_tick.h"
#include "platform/s32k_bindings.h"

#ifndef S32K_TICK_TIMER_COMPARE_VAL
#define S32K_TICK_TIMER_COMPARE_VAL   ((uint16_t)2000U)
#endif
#ifndef S32K_TICK_TICKS_1US
#define S32K_TICK_TICKS_1US           ((uint16_t)4U)
#endif

static volatile uint32_t g_s32kTick500us = 0U;
static volatile uint16_t g_s32kTickOverflowCount = 0U;

void S32kTick_Init(void)
{
    g_s32kTick500us = 0U;
    {
        EmbCriticalState criticalState = 0U;
        EMB_ENTER_CRITICAL(criticalState);
        g_s32kTickOverflowCount = 0U;
        EMB_EXIT_CRITICAL(criticalState);
    }
}

emb_tick_t S32kTick_Get500us(void)
{
    EmbCriticalState criticalState = 0U;
    emb_tick_t tick;

    EMB_ENTER_CRITICAL(criticalState);
    tick = g_s32kTick500us;
    EMB_EXIT_CRITICAL(criticalState);
    return tick;
}

uint32_t S32kTick_GetMs(void)
{
    return (uint32_t)(S32kTick_Get500us() / 2U);
}

void S32kTick_IrqHandler(void)
{
#ifdef S32K_TICK_TIMEOUT_SERVICE_INSTANCE
    LIN_DRV_TimeoutService(S32K_TICK_TIMEOUT_SERVICE_INSTANCE);
#endif
    g_s32kTickOverflowCount++;
    g_s32kTick500us++;
#ifdef S32K_TICK_LPTMR_INSTANCE
    LPTMR_DRV_ClearCompareFlag(S32K_TICK_LPTMR_INSTANCE);
#endif
}

uint32_t S32kLinTimeoutTimeNsCallback(uint32_t *ns)
{
#ifdef S32K_TICK_LPTMR_INSTANCE
    static uint32_t previousCountValue = 0UL;
    uint32_t counterValue;

    counterValue = LPTMR_DRV_GetCounterValueByCount(S32K_TICK_LPTMR_INSTANCE);
    if (ns != 0)
    {
        *ns = ((uint32_t)(counterValue +
                (uint32_t)g_s32kTickOverflowCount * S32K_TICK_TIMER_COMPARE_VAL -
                previousCountValue)) * 1000UL / S32K_TICK_TICKS_1US;
    }

    {
        EmbCriticalState criticalState = 0U;
        EMB_ENTER_CRITICAL(criticalState);
        g_s32kTickOverflowCount = 0U;
        EMB_EXIT_CRITICAL(criticalState);
    }
    previousCountValue = counterValue;
#else
    if (ns != 0)
        *ns = 0UL;
#endif
    return 0UL;
}
