/*
 * LPTMR 기반 시스템 tick 구현부다.
 * 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고,
 * 등록된 소수의 hook에 같은 interrupt를 전달한다.
 */
#include "runtime_tick.h"

#include <stddef.h>

#include "../drivers/tick_hw.h"

#define RUNTIME_TICK_HOOK_COUNT  4U

typedef struct
{
    RuntimeTickHook hook;
    void           *context;
} RuntimeTickHookSlot;

static volatile uint32_t   g_runtime_tick_ms = 0U;
static volatile uint32_t   g_runtime_tick_base_count = 0U;
static volatile uint32_t   g_runtime_tick_us_accumulator = 0U;
static RuntimeTickHookSlot g_runtime_tick_hooks[RUNTIME_TICK_HOOK_COUNT];

static void RuntimeTick_IrqHandler(void);

InfraStatus RuntimeTick_Init(void)
{
    uint32_t index;

    g_runtime_tick_ms = 0U;
    g_runtime_tick_base_count = 0U;
    g_runtime_tick_us_accumulator = 0U;

    for (index = 0U; index < RUNTIME_TICK_HOOK_COUNT; index++)
    {
        g_runtime_tick_hooks[index].hook = NULL;
        g_runtime_tick_hooks[index].context = NULL;
    }

    return TickHw_Init(RuntimeTick_IrqHandler);
}

uint32_t RuntimeTick_GetMs(void)
{
    return g_runtime_tick_ms;
}

uint32_t RuntimeTick_GetBaseCount(void)
{
    return g_runtime_tick_base_count;
}

void RuntimeTick_ClearHooks(void)
{
    uint32_t index;

    for (index = 0U; index < RUNTIME_TICK_HOOK_COUNT; index++)
    {
        g_runtime_tick_hooks[index].hook = NULL;
        g_runtime_tick_hooks[index].context = NULL;
    }
}

InfraStatus RuntimeTick_RegisterHook(RuntimeTickHook hook, void *context)
{
    uint32_t index;

    if (hook == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    for (index = 0U; index < RUNTIME_TICK_HOOK_COUNT; index++)
    {
        if (g_runtime_tick_hooks[index].hook == NULL)
        {
            g_runtime_tick_hooks[index].context = context;
            g_runtime_tick_hooks[index].hook = hook;
            return INFRA_STATUS_OK;
        }
    }

    return INFRA_STATUS_FULL;
}

static void RuntimeTick_IrqHandler(void)
{
    uint32_t index;

    g_runtime_tick_base_count++;
    g_runtime_tick_us_accumulator += RUNTIME_TICK_ISR_PERIOD_US;

    while (g_runtime_tick_us_accumulator >= 1000U)
    {
        g_runtime_tick_ms++;
        g_runtime_tick_us_accumulator -= 1000U;
    }

    for (index = 0U; index < RUNTIME_TICK_HOOK_COUNT; index++)
    {
        if (g_runtime_tick_hooks[index].hook != NULL)
        {
            g_runtime_tick_hooks[index].hook(g_runtime_tick_hooks[index].context);
        }
    }

    TickHw_ClearCompareFlag();
}
