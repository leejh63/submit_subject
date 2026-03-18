#include "runtime.h"

#include <stddef.h>
#include <stdio.h>

#include "runtime_task.h"
#include "runtime_status.h"
#include "runtime_tick.h"
#include "node_uart.h"
#include "uart_service.h"
#include "ctrl_mailbox.h"
#include "ctrl_result_box.h"
#include "ctrl_cmd.h"
#include "peripherals_lpuart_1.h"
#include "sdk_project_config.h"
#include "can_types.h"
#include "can_app.h"

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

#define RUNTIME_CAN_MAX_TX_PER_TICK      2U
#define RUNTIME_CAN_MAX_RESULT_PER_TICK  4U

static uint32_t          g_runtimeLastTickMs = 0U;

static NodeUartContext   g_runtimeUartNode;
static RuntimeStatus     g_runtimeStatus;
static CtrlMailbox       g_runtimeCtrlMailbox;
static CtrlResultBox     g_runtimeCtrlResultBox;
static CanApp            g_runtimeCanApp;

static void Runtime_TaskUart(void *context);
static void Runtime_TaskCan(void *context);
static void Runtime_TaskHeartbeat(void *context);
static void Runtime_TaskRender(void *context);

static void Runtime_UpdateTaskRender(NodeUartContext *node);
static void Runtime_PushResult(const CtrlResult *result);
static void Runtime_PushTextResult(const char *text);

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
    NodeUartContext *node;

    node = (NodeUartContext *)context;
    if (node == NULL)
        return;

    g_runtimeStatus.tmp_check_uart++;
    NodeUart_Task(node);
}

static void Runtime_TaskHeartbeat(void *context)
{
    (void)context;

    g_runtimeStatus.tmp_check_heartbeat++;
    RuntimeStatus_SetHeartbeatAlive(&g_runtimeStatus, 1U);
    RuntimeStatus_SetTickMs(&g_runtimeStatus, RuntimeTick_GetMs());
}

static void Runtime_PushResult(const CtrlResult *result)
{
    if (result == NULL)
        return;

    (void)CtrlResultBox_Push(&g_runtimeCtrlResultBox, result);
}

static void Runtime_PushTextResult(const char *text)
{
    CtrlResult result;

    if (text == NULL)
        return;

    CtrlResult_Clear(&result);
    result.type = CTRL_RESULT_OK;

    (void)snprintf(result.text,
                   sizeof(result.text),
                   "%s",
                   text);

    Runtime_PushResult(&result);
}

static void Runtime_TaskCan(void *context)
{
    CtrlCmd     cmd;
    CtrlResult  result;
    uint8_t     handled;
    uint8_t     popped;
    uint32_t    nowMs;

    (void)context;

    RuntimeStatus_SetCanAlive(&g_runtimeStatus, 0U);
    nowMs = RuntimeTick_GetMs();

    CanApp_Task(&g_runtimeCanApp, nowMs);

    for (handled = 0U; handled < RUNTIME_CAN_MAX_TX_PER_TICK; handled++)
    {
        popped = CtrlMailbox_Pop(&g_runtimeCtrlMailbox, &cmd);
        if (popped == 0U)
            break;

        if (CanApp_SubmitCtrlCmd(&g_runtimeCanApp, &cmd) != 0U)
        {
            RuntimeStatus_SetCanAlive(&g_runtimeStatus, 1U);
            g_runtimeStatus.tmp_check_can++;
        }
        else
        {
            Runtime_PushTextResult("[error] can submit failed");
        }
    }

    CanApp_FlushTx(&g_runtimeCanApp, nowMs);

    for (handled = 0U; handled < RUNTIME_CAN_MAX_RESULT_PER_TICK; handled++)
    {
        popped = CanApp_PopCtrlResult(&g_runtimeCanApp, &result);
        if (popped == 0U)
            break;

        RuntimeStatus_SetCanAlive(&g_runtimeStatus, 1U);
        Runtime_PushResult(&result);
    }
}

static void Runtime_TaskRender(void *context)
{
    NodeUartContext *node;

    node = (NodeUartContext *)context;
    if (node == NULL)
        return;

    Runtime_UpdateTaskRender(node);
    NodeUart_RenderTask(node);
}

static void Runtime_UpdateTaskRender(NodeUartContext *node)
{
    char taskText[NODE_UART_TASK_VIEW_SIZE];

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

    NodeUart_SetTaskText(node, taskText);
}

status_t Runtime_Init(void)
{
    status_t       status;
    NodeUartConfig uartConfig;
    CanAppConfig   canConfig;

    RuntimeStatus_Init(&g_runtimeStatus);

    status = RuntimeTick_Init();
    if (status != STATUS_SUCCESS)
        return status;

    g_runtimeLastTickMs = RuntimeTick_GetMs();

    CtrlMailbox_Init(&g_runtimeCtrlMailbox);
    CtrlResultBox_Init(&g_runtimeCtrlResultBox);

    uartConfig.instance = INST_LPUART_1;
    uartConfig.driverState = &lpUartState1;
    uartConfig.userConfig = &lpuart_1_InitConfig0;
    uartConfig.ctrlMailbox = &g_runtimeCtrlMailbox;
    uartConfig.ctrlResultBox = &g_runtimeCtrlResultBox;
    uartConfig.nodeId = (uint8_t)RUNTIME_NODE_ID;

    status = NodeUart_Init(&g_runtimeUartNode, &uartConfig);
    if (status != STATUS_SUCCESS)
        return status;

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
