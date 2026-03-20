#ifndef APP_UART_CONSOLE_H
#define APP_UART_CONSOLE_H

#include "uart_shared_types.h"
#include "app_ctrl_command_mailbox.h"
#include "app_ctrl_result_box.h"

status_t AppUartConsole_Init(AppUartConsoleContext *node,
                             AppCtrlCommandMailbox *commandMailbox,
                             AppCtrlResultBox *resultBox,
                             uint8_t nodeId);

void AppUartConsole_Task(AppUartConsoleContext *node);
void AppUartConsole_RunRxStage(AppUartConsoleContext *node);
void AppUartConsole_RunInputStage(AppUartConsoleContext *node);
void AppUartConsole_RunResultStage(AppUartConsoleContext *node);
void AppUartConsole_RunRecoverStage(AppUartConsoleContext *node);
void AppUartConsole_RunTxStage(AppUartConsoleContext *node, uint32_t nowMs);
uint8_t AppUartConsole_IsErrorState(const AppUartConsoleContext *node);
void AppUartConsole_RenderTask(AppUartConsoleContext *node);
void AppUartConsole_Clear(AppUartConsoleContext *node);
void AppUartConsole_SetTaskText(AppUartConsoleContext *node, const char *text);
void AppUartConsole_SetResultText(AppUartConsoleContext *node, const char *text);
void AppUartConsole_SetValueText(AppUartConsoleContext *node, const char *text);
status_t AppUartConsole_Recover(AppUartConsoleContext *node);

#endif
