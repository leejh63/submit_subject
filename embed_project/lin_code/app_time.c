// app_time.c
#include "sdk_project_config.h"
#include "app_time.h"

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// LPTMR counters (owned by app_time)
// ============================================================
static volatile uint32_t s_timerOverflowInterruptCount = 0U;
static volatile uint32_t s_tick500us = 0U;

// ============================================================
// Critical section helper (local)
// ============================================================
static inline void ENTER_CRITICAL(void) { INT_SYS_DisableIRQGlobal(); }
static inline void EXIT_CRITICAL(void)  { INT_SYS_EnableIRQGlobal();  }

// ============================================================
// Public API
// ============================================================
uint32_t app_time_now_tick500us(void)
{
    //  read (32-bit aligned, Cortex-M)
    return (uint32_t)s_tick500us;
}

// ============================================================
// ISR (symbol must remain exactly this name)
// ============================================================
void LPTMR_ISR(void)
{
    LIN_DRV_TimeoutService(INST_LIN2);
    s_tick500us++;
    s_timerOverflowInterruptCount++;
    LPTMR_DRV_ClearCompareFlag(INST_LPTMR_1);
}

// ============================================================
// LIN time interval callback (symbol must remain exactly this name)
// ============================================================
uint32_t lin1TimerGetTimeIntervalCallback0(uint32_t *ns)
{
    static uint32_t previousCountValue = 0UL;
    uint32_t counterValue;

    counterValue = LPTMR_DRV_GetCounterValueByCount(INST_LPTMR_1);

    uint32_t ovf;
    ENTER_CRITICAL();
    ovf = s_timerOverflowInterruptCount;
    s_timerOverflowInterruptCount = 0U;
    EXIT_CRITICAL();

    *ns = ((uint32_t)(counterValue + (uint32_t)ovf * TIMER_COMPARE_VAL - previousCountValue))
            * 1000U / TIMER_TICKS_1US;

    previousCountValue = counterValue;
    return 0UL;
}

