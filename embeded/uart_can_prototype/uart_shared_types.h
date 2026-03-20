#ifndef UART_SHARED_TYPES_H
#define UART_SHARED_TYPES_H

#include <stdint.h>
#include "lpuart_driver.h"
#include "msg_queue.h"
#include "app_ctrl_command_mailbox.h"
#include "app_ctrl_result_box.h"

/*
 * UART + AppUartConsole 계층 전체에서 공통으로 사용하는 크기, 상태값, 구조체 모음.
 */

#define UART_SERVICE_RX_BUFFER_SIZE             64U
#define UART_SERVICE_TX_CHUNK_SIZE             MSG_QUEUE_ITEM_DATA_MAX
#define UART_SERVICE_TX_QUEUE_SIZE              8U
#define UART_SERVICE_RX_PENDING_SIZE            32U

#define APP_UART_CONSOLE_LINE_BUFFER_SIZE       64U
#define APP_UART_CONSOLE_INPUT_VIEW_SIZE        48U
#define APP_UART_CONSOLE_TASK_VIEW_SIZE         96U
#define APP_UART_CONSOLE_RESULT_VIEW_SIZE       96U
#define APP_UART_CONSOLE_VALUE_VIEW_SIZE        64U

typedef enum
{
    APP_UART_CONSOLE_STATE_IDLE = 0,
    APP_UART_CONSOLE_STATE_LINE_READY,
    APP_UART_CONSOLE_STATE_PROCESSING,
    APP_UART_CONSOLE_STATE_ERROR
} AppUartConsoleState;

typedef enum
{
    UART_ERROR_NONE = 0,
    UART_ERROR_HW_INIT,
    UART_ERROR_QUEUE_INIT,
    UART_ERROR_RX_PENDING_OVERFLOW,
    UART_ERROR_RX_LINE_OVERFLOW,
    UART_ERROR_TX_QUEUE_FULL,
    UART_ERROR_TX_DRIVER,
    UART_ERROR_TX_TIMEOUT,
    UART_ERROR_RX_DRIVER
} UartErrorCode;

typedef struct
{
    uint8_t             buffer[UART_SERVICE_RX_PENDING_SIZE];
    volatile uint16_t   head;
    volatile uint16_t   tail;
    volatile uint8_t    overflow;
    volatile uint32_t   overflowCount;
} UartRxPendingBuffer;

typedef struct
{
    char      buffer[UART_SERVICE_RX_BUFFER_SIZE];
    uint16_t  length;
    uint8_t   lineReady;
    uint8_t   overflow;
} UartLineBuffer;

typedef struct
{
    char         currentBuffer[UART_SERVICE_TX_CHUNK_SIZE + 1U];
    uint16_t     currentLength;
    uint8_t      busy;
    uint32_t     startTickMs;
    uint32_t     timeoutMs;
    MsgQueue     queue;
    MsgQueueItem queueStorage[UART_SERVICE_TX_QUEUE_SIZE];
} UartTxContext;

typedef struct
{
    UartRxPendingBuffer    rxPending;
    UartLineBuffer         rxLine;
    UartTxContext          tx;
    volatile uint8_t       errorFlag;
    volatile uint32_t      errorCount;
    volatile UartErrorCode errorCode;
} UartChannel;

typedef struct
{
    uint32_t                    instance;
    lpuart_state_t             *driverState;
    const lpuart_user_config_t *userConfig;
    uint8_t                     rxByte;
    UartChannel                 channel;
} UartPort;

typedef struct
{
    char    inputViewText[APP_UART_CONSOLE_INPUT_VIEW_SIZE];
    char    taskViewText[APP_UART_CONSOLE_TASK_VIEW_SIZE];
    char    resultViewText[APP_UART_CONSOLE_RESULT_VIEW_SIZE];
    char    valueViewText[APP_UART_CONSOLE_VALUE_VIEW_SIZE];
    uint8_t fullRefreshRequired;
    uint8_t inputDirty;
    uint8_t taskDirty;
    uint8_t resultDirty;
    uint8_t valueDirty;
    uint8_t layoutDrawn;
} AppUartConsoleView;

typedef struct
{
    UartPort               uart;
    AppUartConsoleState    state;
    char                   commandLineBuffer[APP_UART_CONSOLE_LINE_BUFFER_SIZE];
    uint16_t               commandLineLength;
    AppUartConsoleView     view;
    AppCtrlCommandMailbox *commandMailbox;
    AppCtrlResultBox      *resultBox;
    uint8_t                nodeId;
    uint8_t                errorFlag;
} AppUartConsoleContext;

#endif
