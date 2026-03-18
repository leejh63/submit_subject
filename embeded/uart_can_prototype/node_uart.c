#include "node_uart.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "uart_service.h"
#include "ctrl_input.h"
#include "ctrl_result_box.h"

static void NodeUart_UpdateInputView(NodeUartContext *node);
static void NodeUart_RequestRefresh(NodeUartContext *node);
static void NodeUart_Render(NodeUartContext *node);

static void NodeUart_ProcessInput(NodeUartContext *node);
static void NodeUart_ProcessLine(NodeUartContext *node);
static void NodeUart_ProcessResult(NodeUartContext *node);
static void NodeUart_ClearLineBuffer(NodeUartContext *node);

static const char *NodeUart_GetErrorText(UartErrorCode errorCode);

static const char *NodeUart_GetErrorText(UartErrorCode errorCode)
{
    switch (errorCode)
    {
        case UART_ERROR_HW_INIT:
            return "[error] uart hw init failed";

        case UART_ERROR_QUEUE_INIT:
            return "[error] uart queue init failed";

        case UART_ERROR_TX_DRIVER:
            return "[error] uart tx driver failed";

        case UART_ERROR_TX_TIMEOUT:
            return "[error] uart tx timeout";

        case UART_ERROR_RX_PENDING_OVERFLOW:
            return "[error] uart rx pending overflow";

        case UART_ERROR_RX_LINE_OVERFLOW:
            return "[error] uart rx line overflow";

        case UART_ERROR_TX_QUEUE_FULL:
            return "[error] uart tx queue full";

        case UART_ERROR_NONE:
        default:
            return "[error] uart failure";
    }
}

static void NodeUart_ClearText(char *buffer, uint16_t bufferSize)
{
    if (buffer == NULL || bufferSize == 0U)
        return;

    buffer[0] = '\0';
}

static void NodeUart_ClearLineBuffer(NodeUartContext *node)
{
    if (node == NULL)
        return;

    node->lineBuffer[0] = '\0';
    node->lineLength = 0U;
}

static void NodeUart_RequestRefresh(NodeUartContext *node)
{
    if (node == NULL)
        return;

    node->view.refreshRequired = 1U;
}

void NodeUart_SetResultText(NodeUartContext *node, const char *text)
{
    if (node == NULL || text == NULL)
        return;

    (void)snprintf(node->view.resultLine,
                   sizeof(node->view.resultLine),
                   "%s",
                   text);

    NodeUart_RequestRefresh(node);
}

void NodeUart_SetTaskText(NodeUartContext *node, const char *text)
{
    if (node == NULL || text == NULL)
        return;

    (void)snprintf(node->view.taskLine,
                   sizeof(node->view.taskLine),
                   "%s",
                   text);

    NodeUart_RequestRefresh(node);
}

static void NodeUart_UpdateInputView(NodeUartContext *node)
{
    char inputText[NODE_UART_INPUT_VIEW_SIZE];
    status_t status;

    if (node == NULL)
        return;

    status = UartService_GetCurrentInputText(&node->uart,
                                             inputText,
                                             (uint16_t)sizeof(inputText));
    if (status != STATUS_SUCCESS)
        return;

    if (strncmp(node->view.inputLine,
                inputText,
                sizeof(node->view.inputLine)) != 0)
    {
        (void)snprintf(node->view.inputLine,
                       sizeof(node->view.inputLine),
                       "%s",
                       inputText);

        NodeUart_RequestRefresh(node);
    }
}

static void NodeUart_Render(NodeUartContext *node)
{
    char renderBuffer[UART_TX_BUFFER_SIZE];
    int written;
    status_t status;
    const char *taskText;
    const char *resultText;

    if (node == NULL)
        return;

    taskText = node->view.taskLine;
    resultText = node->view.resultLine;

    if (taskText[0] == '\0')
        taskText = "(no task message)";

    if (resultText[0] == '\0')
        resultText = "(no result)";
    //  일단 무식하게 그리는중 좀더 좋은 방법은 터미넗을 한번 그리고
    //  변경이 되는 위치는 고정이니 터미널 커서를 이동후 그부분만 적절히 변경하면
    //	훨씬 좋을듯! 다만 현재는 일단 큰틀 구현후 세세한 부분을 좀더 다듬을 생
    written = snprintf(renderBuffer,
                       sizeof(renderBuffer),
                       "\033[H"
                       "\033[2K========================\r\n"
                       "\033[2K UART NODE %u CONSOLE\r\n"
                       "\033[2K========================\r\n"
                       "\033[2K\r\n"
                       "\033[2K[INPUT]\r\n"
                       "\033[2Knode%u> %s\r\n"
                       "\033[2K\r\n"
                       "\033[2K[TASK]\r\n"
                       "\033[2K%s\r\n"
                       "\033[2K\r\n"
                       "\033[2K[RESULT]\r\n"
                       "\033[2K%s\r\n"
                       "\033[J",
                       (unsigned int)node->nodeId,
                       (unsigned int)node->nodeId,
                       node->view.inputLine,
                       taskText,
                       resultText);

    if (written <= 0)
        return;

    if ((size_t)written >= sizeof(renderBuffer))
    {
        node->state = NODE_UART_STATE_ERROR;
        node->errorFlag = 1U;
        return;
    }

    status = UartService_RequestTx(&node->uart, renderBuffer);
    if (status == STATUS_SUCCESS)
    {
        node->view.refreshRequired = 0U;
    }
    else if (status == STATUS_BUSY)
    {
        // keep refresh requested
    }
    else
    {
        node->state = NODE_UART_STATE_ERROR;
        node->errorFlag = 1U;
    }
}

status_t NodeUart_Init(NodeUartContext *node,
                       const NodeUartConfig *config)
{
    status_t status;

    if (node == NULL || config == NULL)
        return STATUS_ERROR;

    if (config->driverState == NULL || config->userConfig == NULL)
        return STATUS_ERROR;

    NodeUart_Clear(node);

    node->ctrlMailbox = config->ctrlMailbox;
    node->ctrlResultBox = config->ctrlResultBox;
    node->nodeId = config->nodeId;

    status = UartService_Init(&node->uart,
                              config->instance,
                              config->driverState,
                              config->userConfig);
    if (status != STATUS_SUCCESS)
    {
        node->state = NODE_UART_STATE_ERROR;
        node->errorFlag = 1U;
        return status;
    }

    node->state = NODE_UART_STATE_IDLE;

    NodeUart_ClearText(node->view.inputLine,
                       (uint16_t)sizeof(node->view.inputLine));

    (void)snprintf(node->view.taskLine,
                   sizeof(node->view.taskLine),
                   "system ready / can online");

    (void)snprintf(node->view.resultLine,
                   sizeof(node->view.resultLine),
                   "cmd: help status open <id|all> close <id|all> off <id|all> test <id|all> text <id|all> <msg>");

    NodeUart_RequestRefresh(node);
    return STATUS_SUCCESS;
}

static void NodeUart_ProcessInput(NodeUartContext *node)
{
    status_t status;

    if (node == NULL)
        return;

    if (node->state == NODE_UART_STATE_ERROR)
        return;

    if (UartService_HasLine(&node->uart) == 0U)
        return;

    status = UartService_GetLine(&node->uart,
                                 node->lineBuffer,
                                 UART_LINE_BUFFER_SIZE);
    if (status == STATUS_SUCCESS)
    {
        node->lineLength = (uint16_t)strlen(node->lineBuffer);
        node->state = NODE_UART_STATE_LINE_READY;
    }
    else if (status == STATUS_ERROR)
    {
        node->state = NODE_UART_STATE_ERROR;
        node->errorFlag = 1U;
    }
}

static void NodeUart_ProcessLine(NodeUartContext *node)
{
    CtrlInputResult   result;
    CtrlInputSnapshot snapshot;
    status_t          status;

    if (node == NULL)
        return;

    if (node->state != NODE_UART_STATE_LINE_READY)
        return;

    if (node->ctrlMailbox == NULL)
    {
        node->state = NODE_UART_STATE_ERROR;
        node->errorFlag = 1U;
        NodeUart_SetResultText(node, "[error] ctrl mailbox null");
        return;
    }

    snapshot.nodeState = (uint8_t)node->state;
    snapshot.uartErrorFlag = UartService_HasError(&node->uart);
    snapshot.rxLineLength = node->lineLength;
    snapshot.txBusy = UartService_IsTxBusy(&node->uart);
    snapshot.cmdQueueCount = CtrlMailbox_GetCount(node->ctrlMailbox);
    snapshot.cmdQueueCapacity = CtrlMailbox_GetCapacity(node->ctrlMailbox);

    if (node->ctrlResultBox != NULL)
    {
        snapshot.resultQueueCount = CtrlResultBox_GetCount(node->ctrlResultBox);
        snapshot.resultQueueCapacity = CtrlResultBox_GetCapacity(node->ctrlResultBox);
    }
    else
    {
        snapshot.resultQueueCount = 0U;
        snapshot.resultQueueCapacity = 0U;
    }

    node->state = NODE_UART_STATE_PROCESSING;

    status = CtrlInput_HandleLine(node->lineBuffer,
                                  &snapshot,
                                  node->ctrlMailbox,
                                  &result);

    if (result.text[0] != '\0')
        NodeUart_SetResultText(node, result.text);

    NodeUart_ClearLineBuffer(node);
    node->state = NODE_UART_STATE_IDLE;
    NodeUart_RequestRefresh(node);

    (void)status;
}

static void NodeUart_ProcessResult(NodeUartContext *node)
{
    CtrlResult result;
    uint8_t    popped;

    if (node == NULL)
        return;

    if (node->ctrlResultBox == NULL)
        return;

    popped = CtrlResultBox_Pop(node->ctrlResultBox, &result);
    if (popped == 0U)
        return;

    if (result.text[0] != '\0')
        NodeUart_SetResultText(node, result.text);
}

void NodeUart_Task(NodeUartContext *node)
{
    if (node == NULL)
        return;

    UartService_ProcessRx(&node->uart);

    if (UartService_HasError(&node->uart) != 0U)
    {
        if (node->state != NODE_UART_STATE_ERROR)
        {
            UartErrorCode errorCode;

            errorCode = UartService_GetErrorCode(&node->uart);

            node->state = NODE_UART_STATE_ERROR;
            node->errorFlag = 1U;
            NodeUart_SetResultText(node, NodeUart_GetErrorText(errorCode));
        }
    }

    if (node->state != NODE_UART_STATE_ERROR)
    {
        NodeUart_ProcessInput(node);
        NodeUart_ProcessLine(node);
        NodeUart_ProcessResult(node);
    }
    else
    {
        status_t status;

        status = UartService_GetLine(&node->uart,
                                     node->lineBuffer,
                                     UART_LINE_BUFFER_SIZE);
        if (status == STATUS_SUCCESS)
        {
            node->lineLength = (uint16_t)strlen(node->lineBuffer);

            if (strcmp(node->lineBuffer, "recover") == 0)
                (void)NodeUart_Recover(node);
            else
                NodeUart_SetResultText(node, "[error] uart stopped (type: recover)");

            NodeUart_ClearLineBuffer(node);
            NodeUart_RequestRefresh(node);
        }
    }

    UartService_ProcessTx(&node->uart);
    UartService_UpdateTx(&node->uart);
}

void NodeUart_RenderTask(NodeUartContext *node)
{
    uint16_t txQueueCount;
    uint16_t txQueueCapacity;

    if (node == NULL)
        return;

    if (node->state != NODE_UART_STATE_ERROR)
        NodeUart_UpdateInputView(node);

    if (node->view.refreshRequired == 0U)
        return;

    txQueueCount = UartService_GetTxQueueCount(&node->uart);
    txQueueCapacity = UartService_GetTxQueueCapacity(&node->uart);

    if (txQueueCapacity == 0U)
        return;

    if (txQueueCount < (txQueueCapacity - 1U))
        NodeUart_Render(node);
}

void NodeUart_Clear(NodeUartContext *node)
{
    if (node == NULL)
        return;

    (void)memset(node, 0, sizeof(NodeUartContext));
    node->state = NODE_UART_STATE_IDLE;
}

status_t NodeUart_Recover(NodeUartContext *node)
{
    status_t status;

    if (node == NULL)
        return STATUS_ERROR;

    status = UartService_Recover(&node->uart);
    if (status != STATUS_SUCCESS)
    {
        node->state = NODE_UART_STATE_ERROR;
        node->errorFlag = 1U;
        NodeUart_SetResultText(node, "[error] recover failed");
        return status;
    }

    node->state = NODE_UART_STATE_IDLE;
    node->errorFlag = 0U;

    NodeUart_ClearLineBuffer(node);
    NodeUart_ClearText(node->view.inputLine,
                       (uint16_t)sizeof(node->view.inputLine));

    (void)snprintf(node->view.taskLine,
                   sizeof(node->view.taskLine),
                   "uart recovered");

    (void)snprintf(node->view.resultLine,
                   sizeof(node->view.resultLine),
                   "[ok] uart recover");

    NodeUart_RequestRefresh(node);
    return STATUS_SUCCESS;
}
