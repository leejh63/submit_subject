/*
 * LIN sensor slave 전용 runtime 구현부다.
 * 센서 노드가 실제로 쓰는 task만 등록하여,
 * CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다.
 */
#include "runtime/runtime.h"

#include "app/app_config.h"
#include "infra/runtime_task.h"
#include "infra/runtime_tick.h"
#include "runtime/runtime_io.h"

typedef struct
{
    uint8_t          initialized;
    InfraStatus      init_status;
    AppCore          app;
    RuntimeTaskEntry tasks[4];
} RuntimeContext;

static RuntimeContext g_runtime;

static void Runtime_TaskHeartbeat(void *context, uint32_t now_ms)
{
    AppCore_TaskHeartbeat((AppCore *)context, now_ms);
}

static void Runtime_TaskLinFast(void *context, uint32_t now_ms)
{
    AppCore_TaskLinFast((AppCore *)context, now_ms);
}

static void Runtime_TaskLed(void *context, uint32_t now_ms)
{
    AppCore_TaskLed((AppCore *)context, now_ms);
}

static void Runtime_TaskAdc(void *context, uint32_t now_ms)
{
    AppCore_TaskAdc((AppCore *)context, now_ms);
}

static void Runtime_BuildTaskTable(RuntimeContext *runtime)
{
    runtime->tasks[0].name        = "lin_fast";
    runtime->tasks[0].period_ms   = APP_TASK_LIN_FAST_MS;
    runtime->tasks[0].last_run_ms = 0U;
    runtime->tasks[0].task_fn     = Runtime_TaskLinFast;
    runtime->tasks[0].context     = &runtime->app;

    runtime->tasks[1].name        = "adc";
    runtime->tasks[1].period_ms   = APP_TASK_ADC_MS;
    runtime->tasks[1].last_run_ms = 0U;
    runtime->tasks[1].task_fn     = Runtime_TaskAdc;
    runtime->tasks[1].context     = &runtime->app;

    runtime->tasks[2].name        = "led";
    runtime->tasks[2].period_ms   = APP_TASK_LED_MS;
    runtime->tasks[2].last_run_ms = 0U;
    runtime->tasks[2].task_fn     = Runtime_TaskLed;
    runtime->tasks[2].context     = &runtime->app;

    runtime->tasks[3].name        = "heartbeat";
    runtime->tasks[3].period_ms   = APP_TASK_HEARTBEAT_MS;
    runtime->tasks[3].last_run_ms = 0U;
    runtime->tasks[3].task_fn     = Runtime_TaskHeartbeat;
    runtime->tasks[3].context     = &runtime->app;
}

static void Runtime_FaultLoop(void)
{
    for (;;)
    {
    }
}

InfraStatus Runtime_Init(void)
{
    InfraStatus status;
    uint32_t    start_ms;

    g_runtime.initialized = 0U;
    g_runtime.init_status = INFRA_STATUS_NOT_READY;

    status = RuntimeIo_BoardInit();
    if (status != INFRA_STATUS_OK)
    {
        g_runtime.init_status = status;
        return status;
    }

    status = RuntimeTick_Init();
    if (status != INFRA_STATUS_OK)
    {
        g_runtime.init_status = status;
        return status;
    }

    status = AppCore_Init(&g_runtime.app);
    if (status != INFRA_STATUS_OK)
    {
        g_runtime.init_status = status;
        return status;
    }

    RuntimeTick_ClearHooks();
    status = RuntimeTick_RegisterHook(AppCore_OnTickIsr, &g_runtime.app);
    if (status != INFRA_STATUS_OK)
    {
        g_runtime.init_status = status;
        return status;
    }

    Runtime_BuildTaskTable(&g_runtime);
    start_ms = RuntimeTick_GetMs();
    RuntimeTask_ResetTable(g_runtime.tasks,
                           INFRA_ARRAY_COUNT(g_runtime.tasks),
                           start_ms);

    g_runtime.initialized = 1U;
    g_runtime.init_status = INFRA_STATUS_OK;
    return INFRA_STATUS_OK;
}

void Runtime_Run(void)
{
    if ((g_runtime.initialized == 0U) || (g_runtime.init_status != INFRA_STATUS_OK))
    {
        Runtime_FaultLoop();
    }

    for (;;)
    {
        RuntimeTask_RunDue(g_runtime.tasks,
                           INFRA_ARRAY_COUNT(g_runtime.tasks),
                           RuntimeTick_GetMs());
    }
}

const AppCore *Runtime_GetApp(void)
{
    return &g_runtime.app;
}
