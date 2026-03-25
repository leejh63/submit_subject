/*
 * LPTMR 기반 시스템 tick 구현부다.
 * 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고,
 * 등록된 소수의 hook에 같은 interrupt를 전달한다.
 */
#include "runtime_tick.h"

#include <stddef.h>

#include "interrupt_manager.h"
#include "sdk_project_config.h"

#define RUNTIME_TICK_HOOK_COUNT  4U

/*
 * 등록된 ISR hook 하나를 표현하는 슬롯이다.
 * 각 슬롯은 callback과 opaque context를 짝으로 가지고,
 * tick 계층이 하드코딩 없이 몇 개 모듈에 알릴 수 있게 한다.
 */
typedef struct
{
    RuntimeTickHook hook;
    void           *context;
} RuntimeTickHookSlot;

/*
 * IRQ 경로와 일반 호출자가 공유하는 전역 tick 상태다.
 * 이 카운터들은 millisecond 시간과 base interrupt 횟수를 모두 제공해,
 * 더 세밀한 정보를 원하는 모듈도 활용할 수 있게 한다.
 */
static volatile uint32_t   g_runtime_tick_ms = 0U;
static volatile uint32_t   g_runtime_tick_base_count = 0U;
static volatile uint32_t   g_runtime_tick_us_accumulator = 0U;
static RuntimeTickHookSlot g_runtime_tick_hooks[RUNTIME_TICK_HOOK_COUNT];

/*
 * 시간을 증가시키고 hook을 실행하는 타이머 interrupt handler다.
 * base period를 millisecond로 누적한 뒤,
 * 등록된 리스너를 호출하고 마지막에 타이머 플래그를 지운다.
 */
static void RuntimeTick_IrqHandler(void);

/*
 * 하드웨어 타이머를 올리고 로컬 tick 상태를 초기화한다.
 * hook 슬롯도 여기서 비워서,
 * 펌웨어가 항상 알려진 interrupt 구독 상태로 시작하게 한다.
 */
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

    /*
     * 사용자 바인딩 확인:
     * 이 모듈은 board/peripherals_lptmr_1.* 에서 INST_LPTMR_1 과
     * 500 us 기준 인터럽트용 lptmr_1_config0 을 제공한다고 가정한다.
     */
    LPTMR_DRV_Init(INST_LPTMR_1, &lptmr_1_config0, false);

    INT_SYS_InstallHandler(LPTMR0_IRQn, RuntimeTick_IrqHandler, (isr_t *)NULL);
    INT_SYS_EnableIRQ(LPTMR0_IRQn);
    LPTMR_DRV_StartCounter(INST_LPTMR_1);

    return INFRA_STATUS_OK;
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

/*
 * base tick마다 호출할 가벼운 callback 하나를 등록한다.
 * 작은 고정 hook 테이블을 사용해 ISR fan-out을 예측 가능하게 유지하면서도,
 * LIN 같은 모듈을 느슨하게 결합시킬 수 있다.
 */
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

    LPTMR_DRV_ClearCompareFlag(INST_LPTMR_1);
}
