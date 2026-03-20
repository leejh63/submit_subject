#ifndef APP_UART_CONSOLE_TASK_H
#define APP_UART_CONSOLE_TASK_H

#include <stdint.h>

#include "app_uart_console.h"

// AppUartConsole 위에 얹는 얇은 orchestration 레이어.
// 기본 동작은 유지하되, 이후에는 AppUartConsole step API를 직접 조합해서
// 다른 UART UI / 명령 처리 정책으로 바꾸기 쉽게 만드는 목적이다.

typedef struct
{
    AppUartConsoleContext *node;
} AppUartConsoleTaskContext;

void AppUartConsoleTask_Clear(AppUartConsoleTaskContext *task);
void AppUartConsoleTask_Init(AppUartConsoleTaskContext *task, AppUartConsoleContext *node);
void AppUartConsoleTask_Run(AppUartConsoleTaskContext *task, uint32_t nowMs);
void AppUartConsoleTask_Render(AppUartConsoleTaskContext *task);

#endif
