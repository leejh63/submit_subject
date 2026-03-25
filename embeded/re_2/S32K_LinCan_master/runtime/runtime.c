/*
 * runtime 부트스트랩과 super-loop 스케줄러 구현부다.
 * 이 파일은 보드 초기화, tick 소스, AppCore context,
 * 그리고 모든 역할이 쓰는 주기 task 테이블을 조립한다.
 */
#include "runtime/runtime.h"

#include "app/app_config.h"
#include "infra/runtime_task.h"
#include "infra/runtime_tick.h"
#include "runtime/runtime_io.h"

/*
 * runtime이 소유하는 전체 시스템 context다.
 * 하나의 인스턴스가 초기화 상태와 AppCore 객체,
 * super-loop가 쓰는 주기 task 테이블을 함께 가진다.
 */
typedef struct
{
    uint8_t          initialized;
    InfraStatus      init_status;
    AppCore          app;
    RuntimeTaskEntry tasks[9];
} RuntimeContext;

/*
 * 현재 펌웨어 이미지가 사용하는 단일 runtime 인스턴스다.
 * 전역으로 두면 얇은 entry point와 runtime helper가,
 * 부팅 전체 동안 하나의 application context를 공유할 수 있다.
 */
static RuntimeContext g_runtime;

/*
 * 얇은 task adapter 함수들이다.
 * generic scheduler 시그니처와 역할 인지형 AppCore task를 연결하면서,
 * 스케줄러 세부 구현을 app 계층에 노출하지 않는다.
 */
static void Runtime_TaskHeartbeat(void *context, uint32_t now_ms)
{
    AppCore_TaskHeartbeat((AppCore *)context, now_ms);
}

static void Runtime_TaskUart(void *context, uint32_t now_ms)
{
    AppCore_TaskUart((AppCore *)context, now_ms);
}

static void Runtime_TaskCan(void *context, uint32_t now_ms)
{
    AppCore_TaskCan((AppCore *)context, now_ms);
}

static void Runtime_TaskLinFast(void *context, uint32_t now_ms)
{
    AppCore_TaskLinFast((AppCore *)context, now_ms);
}

static void Runtime_TaskLinPoll(void *context, uint32_t now_ms)
{
    AppCore_TaskLinPoll((AppCore *)context, now_ms);
}

static void Runtime_TaskButton(void *context, uint32_t now_ms)
{
    AppCore_TaskButton((AppCore *)context, now_ms);
}

static void Runtime_TaskLed(void *context, uint32_t now_ms)
{
    AppCore_TaskLed((AppCore *)context, now_ms);
}

static void Runtime_TaskAdc(void *context, uint32_t now_ms)
{
    AppCore_TaskAdc((AppCore *)context, now_ms);
}

static void Runtime_TaskRender(void *context, uint32_t now_ms)
{
    AppCore_TaskRender((AppCore *)context, now_ms);
}

/*
 * super-loop가 사용하는 고정 task 테이블을 채운다.
 * 각 항목은 AppCore task 하나와 실행 주기,
 * 그리고 공유 application context를 함께 연결한다.
 */
static void Runtime_BuildTaskTable(RuntimeContext *runtime)
{
    runtime->tasks[0].name        = "uart";
    runtime->tasks[0].period_ms   = APP_TASK_UART_MS;
    runtime->tasks[0].last_run_ms = 0U;
    runtime->tasks[0].task_fn     = Runtime_TaskUart;
    runtime->tasks[0].context     = &runtime->app;

    runtime->tasks[1].name        = "lin_fast";
    runtime->tasks[1].period_ms   = APP_TASK_LIN_FAST_MS;
    runtime->tasks[1].last_run_ms = 0U;
    runtime->tasks[1].task_fn     = Runtime_TaskLinFast;
    runtime->tasks[1].context     = &runtime->app;

    runtime->tasks[2].name        = "button";
    runtime->tasks[2].period_ms   = APP_TASK_BUTTON_MS;
    runtime->tasks[2].last_run_ms = 0U;
    runtime->tasks[2].task_fn     = Runtime_TaskButton;
    runtime->tasks[2].context     = &runtime->app;

    runtime->tasks[3].name        = "can";
    runtime->tasks[3].period_ms   = APP_TASK_CAN_MS;
    runtime->tasks[3].last_run_ms = 0U;
    runtime->tasks[3].task_fn     = Runtime_TaskCan;
    runtime->tasks[3].context     = &runtime->app;

    runtime->tasks[4].name        = "adc";
    runtime->tasks[4].period_ms   = APP_TASK_ADC_MS;
    runtime->tasks[4].last_run_ms = 0U;
    runtime->tasks[4].task_fn     = Runtime_TaskAdc;
    runtime->tasks[4].context     = &runtime->app;

    runtime->tasks[5].name        = "lin_poll";
    runtime->tasks[5].period_ms   = APP_TASK_LIN_POLL_MS;
    runtime->tasks[5].last_run_ms = 0U;
    runtime->tasks[5].task_fn     = Runtime_TaskLinPoll;
    runtime->tasks[5].context     = &runtime->app;

    runtime->tasks[6].name        = "led";
    runtime->tasks[6].period_ms   = APP_TASK_LED_MS;
    runtime->tasks[6].last_run_ms = 0U;
    runtime->tasks[6].task_fn     = Runtime_TaskLed;
    runtime->tasks[6].context     = &runtime->app;

    runtime->tasks[7].name        = "render";
    runtime->tasks[7].period_ms   = APP_TASK_RENDER_MS;
    runtime->tasks[7].last_run_ms = 0U;
    runtime->tasks[7].task_fn     = Runtime_TaskRender;
    runtime->tasks[7].context     = &runtime->app;

    runtime->tasks[8].name        = "heartbeat";
    runtime->tasks[8].period_ms   = APP_TASK_HEARTBEAT_MS;
    runtime->tasks[8].last_run_ms = 0U;
    runtime->tasks[8].task_fn     = Runtime_TaskHeartbeat;
    runtime->tasks[8].context     = &runtime->app;
}

static void Runtime_FaultLoop(void) { for (;;){ } }

/*
 * 보드 바인딩부터 AppCore까지 runtime 전체를 조립한다.
 * 실패가 발생하면 즉시 반환하여,
 * caller가 무한 scheduling loop에 들어가기 전에 멈출 수 있게 한다.
 */
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

/*
 * 무한 cooperative scheduler에 진입한다.
 * 초기화가 성공한 뒤에는 이 루프가 반복적으로 due task를 확인하며,
 * 정상 동작 중에는 반환하지 않는다.
 */
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

/*
 * 진단을 위해 현재 AppCore context를 노출한다.
 * scheduler를 직접 소유하지 않는 외부 코드가,
 * runtime 상태를 읽기 전용으로 확인할 때 유용하다.
 */
const AppCore *Runtime_GetApp(void)
{
    return &g_runtime.app;
}
