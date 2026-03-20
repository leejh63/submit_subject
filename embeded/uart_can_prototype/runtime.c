#include "runtime.h"

#include <stddef.h>

#include "runtime_task.h"
#include "runtime_status.h"
#include "runtime_tick.h"
#include "app_uart_console.h"
#include "app_uart_console_task.h"
#include "uart_service.h"
#include "app_ctrl_command_mailbox.h"
#include "app_ctrl_result_box.h"
#include "peripherals_lpuart_1.h"
#include "sdk_project_config.h"
#include "can_types.h"
#include "can_app.h"
#include "can_task.h"
// 임시구조
#define RUNTIME_ROLE_MASTER      1U
#define RUNTIME_ROLE_SLAVE       2U
#define RUNTIME_ROLE_SLAVETWO    3U

#define RUNTIME_ROLE             RUNTIME_ROLE_MASTER

#if (RUNTIME_ROLE == RUNTIME_ROLE_MASTER)
#define RUNTIME_NODE_ID              1U
#define RUNTIME_DEFAULT_TARGET_ID    CAN_NODE_ID_INVALID
#define RUNTIME_APP_ROLE             CAN_APP_ROLE_MASTER
#elif (RUNTIME_ROLE == RUNTIME_ROLE_SLAVE)
#define RUNTIME_NODE_ID              2U
#define RUNTIME_DEFAULT_TARGET_ID    CAN_NODE_ID_INVALID
#define RUNTIME_APP_ROLE             CAN_APP_ROLE_SLAVE
#elif (RUNTIME_ROLE == RUNTIME_ROLE_SLAVETWO)
#define RUNTIME_NODE_ID              3U
#define RUNTIME_DEFAULT_TARGET_ID    CAN_NODE_ID_INVALID
#define RUNTIME_APP_ROLE             CAN_APP_ROLE_SLAVE_TWO
#else
#error Invalid RUNTIME_ROLE
#endif

static uint32_t          g_runtimeLastTickMs = 0U;

static AppUartConsoleContext   g_runtimeUartNode;
static RuntimeStatus     g_runtimeStatus;
static AppCtrlCommandMailbox       g_runtimeAppCtrlCommandMailbox;
static AppCtrlResultBox     g_runtimeAppCtrlResultBox;
static CanApp            g_runtimeCanApp;
static AppUartConsoleTaskContext   g_runtimeAppUartConsoleTask;
static CanTaskContext    g_runtimeCanTask;

static void Runtime_TaskUart(void *context);
static void Runtime_TaskCan(void *context);
static void Runtime_TaskHeartbeat(void *context);
static void Runtime_TaskRender(void *context);

static void Runtime_UpdateTaskRender(AppUartConsoleContext *node);

static RuntimeTaskEntry g_runtimeTaskTable[] =
{
    {
        "uart",
        1U,
        0U,
        Runtime_TaskUart,
        &g_runtimeUartNode
    },
    {
        "heartbeat",
        1000U,
        0U,
        Runtime_TaskHeartbeat,
        NULL
    },
    {
        "can",
        10U,
        0U,
        Runtime_TaskCan,
        NULL
    },
    {
        "render",
        100U,
        0U,
        Runtime_TaskRender,
        &g_runtimeUartNode
    },
};

#define RUNTIME_TASK_COUNT \
    ((uint32_t)(sizeof(g_runtimeTaskTable) / sizeof(g_runtimeTaskTable[0])))

static void Runtime_TaskUart(void *context)
{
    AppUartConsoleContext *node;

    node = (AppUartConsoleContext *)context;
    if (node == NULL)
        return;

    g_runtimeStatus.tmp_check_uart++;
    AppUartConsoleTask_Run(&g_runtimeAppUartConsoleTask, RuntimeTick_GetMs());
}

static void Runtime_TaskHeartbeat(void *context)
{
    (void)context;

    g_runtimeStatus.tmp_check_heartbeat++;
    RuntimeStatus_SetHeartbeatAlive(&g_runtimeStatus, 1U);
    RuntimeStatus_SetTickMs(&g_runtimeStatus, RuntimeTick_GetMs());
}

static void Runtime_TaskCan(void *context)
{
    uint32_t nowMs;
    uint8_t  activity;

    (void)context;

    nowMs = RuntimeTick_GetMs();
    activity = CanTask_Run(&g_runtimeCanTask, nowMs);

    RuntimeStatus_SetCanAlive(&g_runtimeStatus, activity);
    if (activity != 0U)
        g_runtimeStatus.tmp_check_can++;
}

static void Runtime_TaskRender(void *context)
{
    AppUartConsoleContext *node;

    node = (AppUartConsoleContext *)context;
    if (node == NULL)
        return;

    Runtime_UpdateTaskRender(node);
    AppUartConsoleTask_Render(&g_runtimeAppUartConsoleTask);
}

static void Runtime_UpdateTaskRender(AppUartConsoleContext *node)
{
    char taskText[APP_UART_CONSOLE_TASK_VIEW_SIZE];

    if (node == NULL)
        return;

    if (UartService_HasError(&node->uart) != 0U)
        RuntimeStatus_SetUartOk(&g_runtimeStatus, 0U);
    else
        RuntimeStatus_SetUartOk(&g_runtimeStatus, 1U);

    RuntimeStatus_SetTickMs(&g_runtimeStatus, RuntimeTick_GetMs());

    RuntimeStatus_BuildTaskText(&g_runtimeStatus,
                                taskText,
                                (uint16_t)sizeof(taskText));

    AppUartConsole_SetTaskText(node, taskText);
}

status_t Runtime_Init(void)
{
    status_t       status;
    AppUartConsoleConfig uartConfig;
    CanAppConfig   canConfig;

    RuntimeStatus_Init(&g_runtimeStatus);

    status = RuntimeTick_Init();
    if (status != STATUS_SUCCESS)
        return status;

    g_runtimeLastTickMs = RuntimeTick_GetMs();

    AppCtrlCommandMailbox_Init(&g_runtimeAppCtrlCommandMailbox);
    AppCtrlResultBox_Init(&g_runtimeAppCtrlResultBox);

    // 질문이거 AppUartConsole_Init 내부로 넣는게 좋을듯?>
    uartConfig.instance = INST_LPUART_1;
    uartConfig.driverState = &lpUartState1;
    uartConfig.userConfig = &lpuart_1_InitConfig0;
    uartConfig.commandMailbox = &g_runtimeAppCtrlCommandMailbox;
    uartConfig.resultBox = &g_runtimeAppCtrlResultBox;
    uartConfig.nodeId = (uint8_t)RUNTIME_NODE_ID;

    status = AppUartConsole_Init(&g_runtimeUartNode, &uartConfig);
    if (status != STATUS_SUCCESS)
        return status;
    // 질문이거 CanApp_Init 내부로 넣는게 좋을듯?>
    canConfig.localNodeId = (uint8_t)RUNTIME_NODE_ID;
    canConfig.role = (uint8_t)RUNTIME_APP_ROLE;
    canConfig.defaultTargetNodeId = (uint8_t)RUNTIME_DEFAULT_TARGET_ID;
    canConfig.instance = INST_FLEXCAN_CONFIG_1;
    canConfig.txMbIndex = CAN_HW_TX_MB_INDEX;
    canConfig.rxMbIndex = CAN_HW_RX_MB_INDEX;
    canConfig.defaultTimeoutMs = 300U;
    canConfig.driverState = &flexcanState0;
    canConfig.userConfig = &flexcanInitConfig0;

    if (CanApp_Init(&g_runtimeCanApp, &canConfig) == 0U)
        return STATUS_ERROR;

    AppUartConsoleTask_Init(&g_runtimeAppUartConsoleTask, &g_runtimeUartNode);
    CanTask_Init(&g_runtimeCanTask,
                 &g_runtimeCanApp,
                 &g_runtimeAppCtrlCommandMailbox,
                 &g_runtimeAppCtrlResultBox);

    Runtime_UpdateTaskRender(&g_runtimeUartNode);

    return STATUS_SUCCESS;
}

void Runtime_Run(void)
{
    uint32_t currentTickMs;

    currentTickMs = RuntimeTick_GetMs();

    while (g_runtimeLastTickMs != currentTickMs)
    {
        g_runtimeLastTickMs++;

        RuntimeTask_RunTable(g_runtimeTaskTable,
                             RUNTIME_TASK_COUNT,
                             g_runtimeLastTickMs);
    }
}

void Runtime_FaultLoop(void)
{
    for (;;)
    {
        // fault loop
    }
}
