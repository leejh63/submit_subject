#include "app_uart_console.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "uart_service.h"
#include "app_ctrl_input.h"
#include "app_ctrl_result_box.h"
#include "runtime_tick.h"

#define APP_UART_CONSOLE_ROW_INPUT        6U
#define APP_UART_CONSOLE_ROW_TASK_LINE1   9U
#define APP_UART_CONSOLE_ROW_TASK_LINE2  10U
#define APP_UART_CONSOLE_ROW_TASK_LINE3  11U
#define APP_UART_CONSOLE_ROW_RESULT      14U
#define APP_UART_CONSOLE_ROW_VALUE       17U
#define APP_UART_CONSOLE_RENDER_BUFFER_SIZE  256U

static void AppUartConsole_UpdateInputView(AppUartConsoleContext *node);
static void AppUartConsole_RequestFullRefresh(AppUartConsoleContext *node);
static void AppUartConsole_RequestInputRefresh(AppUartConsoleContext *node);
static void AppUartConsole_RequestTaskRefresh(AppUartConsoleContext *node);
static void AppUartConsole_RequestResultRefresh(AppUartConsoleContext *node);
static void AppUartConsole_RequestValueRefresh(AppUartConsoleContext *node);
static void AppUartConsole_RenderLayout(AppUartConsoleContext *node);
static void AppUartConsole_RenderInputLine(AppUartConsoleContext *node);
static void AppUartConsole_RenderTaskLines(AppUartConsoleContext *node);
static void AppUartConsole_RenderResultLine(AppUartConsoleContext *node);
static void AppUartConsole_RenderValueLine(AppUartConsoleContext *node);
static void AppUartConsole_ProcessInput(AppUartConsoleContext *node);
static void AppUartConsole_ProcessLine(AppUartConsoleContext *node);
static void AppUartConsole_ProcessResult(AppUartConsoleContext *node);
static void AppUartConsole_ClearLineBuffer(AppUartConsoleContext *node);
static void AppUartConsole_SetViewText(char *dst,
                                       uint16_t dstSize,
                                       const char *src,
                                       uint8_t *outChanged);
static void AppUartConsole_ExtractTextLine(const char *src,
                                           uint8_t lineIndex,
                                           char *outBuffer,
                                           uint16_t outBufferSize);
static const char *AppUartConsole_GetErrorText(UartErrorCode errorCode);

static const char *AppUartConsole_GetErrorText(UartErrorCode errorCode)
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
        case UART_ERROR_RX_DRIVER:
            return "[error] uart rx driver failed";
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

static void AppUartConsole_SetViewText(char *dst,
                                       uint16_t dstSize,
                                       const char *src,
                                       uint8_t *outChanged)
{
    char temp[APP_UART_CONSOLE_RENDER_BUFFER_SIZE];

    if ((dst == NULL) || (src == NULL) || (outChanged == NULL) || (dstSize == 0U))
        return;

    (void)snprintf(temp, sizeof(temp), "%s", src);

    if (strncmp(dst, temp, dstSize) == 0)
    {
        *outChanged = 0U;
        return;
    }

    (void)snprintf(dst, dstSize, "%s", temp);
    *outChanged = 1U;
}

static void AppUartConsole_ExtractTextLine(const char *src,
                                           uint8_t lineIndex,
                                           char *outBuffer,
                                           uint16_t outBufferSize)
{
    uint8_t currentLine;
    uint16_t outIndex;

    if ((src == NULL) || (outBuffer == NULL) || (outBufferSize == 0U))
        return;

    currentLine = 0U;
    outIndex = 0U;
    outBuffer[0] = '\0';

    while (*src != '\0')
    {
        if ((currentLine == lineIndex) && (*src != '\r') && (*src != '\n'))
        {
            if (outIndex < (uint16_t)(outBufferSize - 1U))
            {
                outBuffer[outIndex] = *src;
                outIndex++;
            }
        }

        if ((*src == '\r') || (*src == '\n'))
        {
            if ((*src == '\r') && (*(src + 1) == '\n'))
                src++;

            if (currentLine == lineIndex)
                break;

            currentLine++;
        }

        src++;
    }

    outBuffer[outIndex] = '\0';
}

static void AppUartConsole_ClearText(char *buffer, uint16_t bufferSize)
{
    if ((buffer == NULL) || (bufferSize == 0U))
        return;

    buffer[0] = '\0';
}

static void AppUartConsole_ClearLineBuffer(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    node->commandLineBuffer[0] = '\0';
    node->commandLineLength = 0U;
}

static void AppUartConsole_RequestFullRefresh(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    node->view.fullRefreshRequired = 1U;
    node->view.inputDirty = 1U;
    node->view.taskDirty = 1U;
    node->view.resultDirty = 1U;
    node->view.valueDirty = 1U;
}

static void AppUartConsole_RequestInputRefresh(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    node->view.inputDirty = 1U;
}

static void AppUartConsole_RequestTaskRefresh(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    node->view.taskDirty = 1U;
}

static void AppUartConsole_RequestResultRefresh(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    node->view.resultDirty = 1U;
}

static void AppUartConsole_RequestValueRefresh(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    node->view.valueDirty = 1U;
}

void AppUartConsole_SetResultText(AppUartConsoleContext *node, const char *text)
{
    uint8_t changed;

    if ((node == NULL) || (text == NULL))
        return;

    AppUartConsole_SetViewText(node->view.resultViewText,
                               (uint16_t)sizeof(node->view.resultViewText),
                               text,
                               &changed);
    if (changed != 0U)
        AppUartConsole_RequestResultRefresh(node);
}

void AppUartConsole_SetTaskText(AppUartConsoleContext *node, const char *text)
{
    uint8_t changed;

    if ((node == NULL) || (text == NULL))
        return;

    AppUartConsole_SetViewText(node->view.taskViewText,
                               (uint16_t)sizeof(node->view.taskViewText),
                               text,
                               &changed);
    if (changed != 0U)
        AppUartConsole_RequestTaskRefresh(node);
}

void AppUartConsole_SetValueText(AppUartConsoleContext *node, const char *text)
{
    uint8_t changed;

    if ((node == NULL) || (text == NULL))
        return;

    AppUartConsole_SetViewText(node->view.valueViewText,
                               (uint16_t)sizeof(node->view.valueViewText),
                               text,
                               &changed);
    if (changed != 0U)
        AppUartConsole_RequestValueRefresh(node);
}

static void AppUartConsole_UpdateInputView(AppUartConsoleContext *node)
{
    char inputText[APP_UART_CONSOLE_INPUT_VIEW_SIZE];
    uint8_t changed;
    status_t status;

    if (node == NULL)
        return;

    status = UartService_GetCurrentInputText(&node->uart,
                                             inputText,
                                             (uint16_t)sizeof(inputText));
    if (status != STATUS_SUCCESS)
        return;

    AppUartConsole_SetViewText(node->view.inputViewText,
                               (uint16_t)sizeof(node->view.inputViewText),
                               inputText,
                               &changed);
    if (changed != 0U)
        AppUartConsole_RequestInputRefresh(node);
}

static void AppUartConsole_RenderLayout(AppUartConsoleContext *node)
{
    char renderBuffer[APP_UART_CONSOLE_RENDER_BUFFER_SIZE];
    int written;
    status_t status;

    if (node == NULL)
        return;

    written = snprintf(renderBuffer,
                       sizeof(renderBuffer),
                       "\033[2J\033[H"
                       "========================\r\n"
                       " UART NODE %u CONSOLE\r\n"
                       "========================\r\n"
                       "\r\n"
                       "[INPUT]\r\n"
                       "\r\n"
                       "\r\n"
                       "[TASK]\r\n"
                       "\r\n"
                       "\r\n"
                       "\r\n"
                       "\r\n"
                       "[RESULT]\r\n"
                       "\r\n"
                       "\r\n"
                       "[VALUE]\r\n"
                       "\r\n"
                       "\033[J",
                       (unsigned int)node->nodeId);
    if ((written <= 0) || ((size_t)written >= sizeof(renderBuffer)))
    {
        node->state = APP_UART_CONSOLE_STATE_ERROR;
        node->errorFlag = 1U;
        return;
    }

    status = UartService_RequestTx(&node->uart, renderBuffer);
    if (status == STATUS_SUCCESS)
        node->view.layoutDrawn = 1U;
    else if (status != STATUS_BUSY)
    {
        node->state = APP_UART_CONSOLE_STATE_ERROR;
        node->errorFlag = 1U;
    }
}

static void AppUartConsole_RenderInputLine(AppUartConsoleContext *node)
{
    char renderBuffer[APP_UART_CONSOLE_RENDER_BUFFER_SIZE];
    int written;

    if (node == NULL)
        return;

    written = snprintf(renderBuffer,
                       sizeof(renderBuffer),
                       "\033[%u;1H\033[2Knode%u> %s",
                       (unsigned int)APP_UART_CONSOLE_ROW_INPUT,
                       (unsigned int)node->nodeId,
                       node->view.inputViewText);
    if ((written > 0) && ((size_t)written < sizeof(renderBuffer)) &&
        (UartService_RequestTx(&node->uart, renderBuffer) == STATUS_SUCCESS))
    {
        node->view.inputDirty = 0U;
    }
}

static void AppUartConsole_RenderTaskLines(AppUartConsoleContext *node)
{
    char renderBuffer[APP_UART_CONSOLE_RENDER_BUFFER_SIZE];
    char line1[40];
    char line2[40];
    char line3[40];
    const char *taskText;
    int written;

    if (node == NULL)
        return;

    taskText = node->view.taskViewText;
    if (taskText[0] == '\0')
        taskText = "(no task message)";

    AppUartConsole_ExtractTextLine(taskText, 0U, line1, (uint16_t)sizeof(line1));
    AppUartConsole_ExtractTextLine(taskText, 1U, line2, (uint16_t)sizeof(line2));
    AppUartConsole_ExtractTextLine(taskText, 2U, line3, (uint16_t)sizeof(line3));

    written = snprintf(renderBuffer,
                       sizeof(renderBuffer),
                       "\033[%u;1H\033[2K%s"
                       "\033[%u;1H\033[2K%s"
                       "\033[%u;1H\033[2K%s",
                       (unsigned int)APP_UART_CONSOLE_ROW_TASK_LINE1,
                       line1,
                       (unsigned int)APP_UART_CONSOLE_ROW_TASK_LINE2,
                       line2,
                       (unsigned int)APP_UART_CONSOLE_ROW_TASK_LINE3,
                       line3);
    if ((written > 0) && ((size_t)written < sizeof(renderBuffer)) &&
        (UartService_RequestTx(&node->uart, renderBuffer) == STATUS_SUCCESS))
    {
        node->view.taskDirty = 0U;
    }
}

static void AppUartConsole_RenderResultLine(AppUartConsoleContext *node)
{
    char renderBuffer[APP_UART_CONSOLE_RENDER_BUFFER_SIZE];
    const char *resultText;
    int written;

    if (node == NULL)
        return;

    resultText = node->view.resultViewText;
    if (resultText[0] == '\0')
        resultText = "(no result)";

    written = snprintf(renderBuffer,
                       sizeof(renderBuffer),
                       "\033[%u;1H\033[2K%s",
                       (unsigned int)APP_UART_CONSOLE_ROW_RESULT,
                       resultText);
    if ((written > 0) && ((size_t)written < sizeof(renderBuffer)) &&
        (UartService_RequestTx(&node->uart, renderBuffer) == STATUS_SUCCESS))
    {
        node->view.resultDirty = 0U;
    }
}

static void AppUartConsole_RenderValueLine(AppUartConsoleContext *node)
{
    char renderBuffer[APP_UART_CONSOLE_RENDER_BUFFER_SIZE];
    const char *valueText;
    int written;

    if (node == NULL)
        return;

    valueText = node->view.valueViewText;
    if (valueText[0] == '\0')
        valueText = "(reserved)";

    written = snprintf(renderBuffer,
                       sizeof(renderBuffer),
                       "\033[%u;1H\033[2K%s",
                       (unsigned int)APP_UART_CONSOLE_ROW_VALUE,
                       valueText);
    if ((written > 0) && ((size_t)written < sizeof(renderBuffer)) &&
        (UartService_RequestTx(&node->uart, renderBuffer) == STATUS_SUCCESS))
    {
        node->view.valueDirty = 0U;
    }
}

status_t AppUartConsole_Init(AppUartConsoleContext *node,
                             const AppUartConsoleConfig *config)
{
    status_t status;

    if ((node == NULL) || (config == NULL))
        return STATUS_ERROR;

    if ((config->driverState == NULL) || (config->userConfig == NULL))
        return STATUS_ERROR;

    AppUartConsole_Clear(node);
    node->commandMailbox = config->commandMailbox;
    node->resultBox = config->resultBox;
    node->nodeId = config->nodeId;

    status = UartService_Init(&node->uart,
                              config->instance,
                              config->driverState,
                              config->userConfig);
    if (status != STATUS_SUCCESS)
    {
        node->state = APP_UART_CONSOLE_STATE_ERROR;
        node->errorFlag = 1U;
        return status;
    }

    node->state = APP_UART_CONSOLE_STATE_IDLE;
    AppUartConsole_ClearText(node->view.inputViewText, (uint16_t)sizeof(node->view.inputViewText));
    (void)snprintf(node->view.taskViewText,
                   sizeof(node->view.taskViewText),
                   "system ready / can online");
    (void)snprintf(node->view.resultViewText,
                   sizeof(node->view.resultViewText),
                   "cmd: help status open close off test text");
    (void)snprintf(node->view.valueViewText,
                   sizeof(node->view.valueViewText),
                   "value: (not set)");

    AppUartConsole_RequestFullRefresh(node);
    return STATUS_SUCCESS;
}

static void AppUartConsole_ProcessInput(AppUartConsoleContext *node)
{
    status_t status;

    if (node == NULL)
        return;

    if (node->state == APP_UART_CONSOLE_STATE_ERROR)
        return;

    if (UartService_HasLine(&node->uart) == 0U)
        return;

    status = UartService_GetLine(&node->uart,
                                 node->commandLineBuffer,
                                 APP_UART_CONSOLE_LINE_BUFFER_SIZE);
    if (status == STATUS_SUCCESS)
    {
        node->commandLineLength = (uint16_t)strlen(node->commandLineBuffer);
        node->state = APP_UART_CONSOLE_STATE_LINE_READY;
        AppUartConsole_RequestInputRefresh(node);
    }
    else if (status == STATUS_ERROR)
    {
        node->state = APP_UART_CONSOLE_STATE_ERROR;
        node->errorFlag = 1U;
    }
}

static void AppUartConsole_ProcessLine(AppUartConsoleContext *node)
{
    AppCtrlInputResult   result;
    AppCtrlInputSnapshot snapshot;
    status_t             status;

    if (node == NULL)
        return;

    if (node->state != APP_UART_CONSOLE_STATE_LINE_READY)
        return;

    if (node->commandMailbox == NULL)
    {
        node->state = APP_UART_CONSOLE_STATE_ERROR;
        node->errorFlag = 1U;
        AppUartConsole_SetResultText(node, "[error] ctrl mailbox null");
        return;
    }

    snapshot.consoleState = (uint8_t)node->state;
    snapshot.uartErrorFlag = UartService_HasError(&node->uart);
    snapshot.rxLineLength = node->commandLineLength;
    snapshot.txBusy = UartService_IsTxBusy(&node->uart);
    snapshot.commandQueueCount = AppCtrlCommandMailbox_GetCount(node->commandMailbox);
    snapshot.commandQueueCapacity = AppCtrlCommandMailbox_GetCapacity(node->commandMailbox);
    snapshot.resultQueueCount = AppCtrlResultBox_GetCount(node->resultBox);
    snapshot.resultQueueCapacity = AppCtrlResultBox_GetCapacity(node->resultBox);

    node->state = APP_UART_CONSOLE_STATE_PROCESSING;
    status = AppCtrlInput_HandleLine(node->commandLineBuffer,
                                     &snapshot,
                                     node->commandMailbox,
                                     &result);
    if (result.text[0] != '\0')
        AppUartConsole_SetResultText(node, result.text);

    AppUartConsole_SetValueText(node, node->commandLineBuffer);
    AppUartConsole_ClearLineBuffer(node);
    node->state = APP_UART_CONSOLE_STATE_IDLE;
    AppUartConsole_RequestInputRefresh(node);

    (void)status;
}

static void AppUartConsole_ProcessResult(AppUartConsoleContext *node)
{
    AppCtrlResult result;
    uint8_t popped;

    if ((node == NULL) || (node->resultBox == NULL))
        return;

    popped = AppCtrlResultBox_Pop(node->resultBox, &result);
    if (popped == 0U)
        return;

    if (result.text[0] != '\0')
        AppUartConsole_SetResultText(node, result.text);
}

uint8_t AppUartConsole_IsErrorState(const AppUartConsoleContext *node)
{
    if (node == NULL)
        return 1U;

    if (node->state == APP_UART_CONSOLE_STATE_ERROR)
        return 1U;

    if (UartService_HasError(&node->uart) != 0U)
        return 1U;

    return 0U;
}

void AppUartConsole_RunRxStage(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    UartService_ProcessRx(&node->uart);
    if (UartService_HasError(&node->uart) != 0U)
    {
        if (node->state != APP_UART_CONSOLE_STATE_ERROR)
        {
            UartErrorCode errorCode;

            errorCode = UartService_GetErrorCode(&node->uart);
            node->state = APP_UART_CONSOLE_STATE_ERROR;
            node->errorFlag = 1U;
            AppUartConsole_SetResultText(node, AppUartConsole_GetErrorText(errorCode));
        }
    }
}

void AppUartConsole_RunInputStage(AppUartConsoleContext *node)
{
    if ((node == NULL) || (AppUartConsole_IsErrorState(node) != 0U))
        return;

    AppUartConsole_ProcessInput(node);
    AppUartConsole_ProcessLine(node);
}

void AppUartConsole_RunResultStage(AppUartConsoleContext *node)
{
    if ((node == NULL) || (AppUartConsole_IsErrorState(node) != 0U))
        return;

    AppUartConsole_ProcessResult(node);
}

void AppUartConsole_RunRecoverStage(AppUartConsoleContext *node)
{
    status_t status;

    if ((node == NULL) || (AppUartConsole_IsErrorState(node) == 0U))
        return;

    status = UartService_GetLine(&node->uart,
                                 node->commandLineBuffer,
                                 APP_UART_CONSOLE_LINE_BUFFER_SIZE);
    if (status != STATUS_SUCCESS)
        return;

    node->commandLineLength = (uint16_t)strlen(node->commandLineBuffer);
    if (strcmp(node->commandLineBuffer, "recover") == 0)
        (void)AppUartConsole_Recover(node);
    else
        AppUartConsole_SetResultText(node, "[error] uart stopped (type: recover)");

    AppUartConsole_ClearLineBuffer(node);
    AppUartConsole_RequestInputRefresh(node);
}

void AppUartConsole_RunTxStage(AppUartConsoleContext *node, uint32_t nowMs)
{
    if (node == NULL)
        return;

    UartService_ProcessTxWithTick(&node->uart, nowMs);
    UartService_UpdateTxWithTick(&node->uart, nowMs);
}

void AppUartConsole_Task(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    AppUartConsole_RunRxStage(node);
    if (AppUartConsole_IsErrorState(node) == 0U)
    {
        AppUartConsole_RunInputStage(node);
        AppUartConsole_RunResultStage(node);
    }
    else
    {
        AppUartConsole_RunRecoverStage(node);
    }

    AppUartConsole_RunTxStage(node, RuntimeTick_GetMs());
}

void AppUartConsole_RenderTask(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    if (node->state != APP_UART_CONSOLE_STATE_ERROR)
        AppUartConsole_UpdateInputView(node);

    if (node->view.layoutDrawn == 0U || node->view.fullRefreshRequired != 0U)
    {
        AppUartConsole_RenderLayout(node);
        if (node->view.layoutDrawn == 0U)
            return;

        node->view.fullRefreshRequired = 0U;
    }

    if (node->view.inputDirty != 0U)
        AppUartConsole_RenderInputLine(node);
    if (node->view.taskDirty != 0U)
        AppUartConsole_RenderTaskLines(node);
    if (node->view.resultDirty != 0U)
        AppUartConsole_RenderResultLine(node);
    if (node->view.valueDirty != 0U)
        AppUartConsole_RenderValueLine(node);
}

void AppUartConsole_Clear(AppUartConsoleContext *node)
{
    if (node == NULL)
        return;

    (void)memset(node, 0, sizeof(AppUartConsoleContext));
    node->state = APP_UART_CONSOLE_STATE_IDLE;
}

status_t AppUartConsole_Recover(AppUartConsoleContext *node)
{
    status_t status;

    if (node == NULL)
        return STATUS_ERROR;

    status = UartService_Recover(&node->uart);
    if (status != STATUS_SUCCESS)
    {
        node->state = APP_UART_CONSOLE_STATE_ERROR;
        node->errorFlag = 1U;
        return status;
    }

    node->state = APP_UART_CONSOLE_STATE_IDLE;
    node->errorFlag = 0U;
    AppUartConsole_SetTaskText(node, "system recovered");
    AppUartConsole_SetResultText(node, "[ok] uart recover complete");
    AppUartConsole_RequestFullRefresh(node);
    return STATUS_SUCCESS;
}
