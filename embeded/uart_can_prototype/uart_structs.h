#ifndef UART_STRUCTS_H
#define UART_STRUCTS_H

#include <stdint.h>
#include "lpuart_driver.h"
#include "msg_queue.h"
#include "ctrl_mailbox.h"
#include "ctrl_result_box.h"

// ============================================================
// UART 관련 기본 크기
// ============================================================

#define UART_RX_BUFFER_SIZE         64U
#define UART_TX_BUFFER_SIZE         512U
#define UART_LINE_BUFFER_SIZE       64U
#define UART_TX_QUEUE_SIZE          4U
#define UART_RX_PENDING_SIZE        32U

#define NODE_UART_INPUT_VIEW_SIZE   64U
#define NODE_UART_TASK_VIEW_SIZE    96U
#define NODE_UART_RESULT_VIEW_SIZE  96U

// ============================================================
// Node 상태
// ============================================================

typedef enum
{
    NODE_UART_STATE_IDLE = 0,
    NODE_UART_STATE_LINE_READY,
    NODE_UART_STATE_PROCESSING,
    NODE_UART_STATE_ERROR
} NodeUartState;

typedef enum
{
    UART_ERROR_NONE = 0,
	UART_ERROR_HW_INIT,
	UART_ERROR_QUEUE_INIT,
    UART_ERROR_RX_PENDING_OVERFLOW,
    UART_ERROR_RX_LINE_OVERFLOW,
    UART_ERROR_TX_QUEUE_FULL,
    UART_ERROR_TX_DRIVER,
    UART_ERROR_TX_TIMEOUT
} UartErrorCode;

// ============================================================
// RX pending ring buffer
// - ISR: tail만 갱신
// - Main: head만 갱신
// ============================================================

typedef struct
{
    uint8_t             buffer[UART_RX_PENDING_SIZE];
    volatile uint16_t   head;
    volatile uint16_t   tail;
    volatile uint8_t    overflow;
} UartRxPendingBuffer;

// ============================================================
// UART line editor 상태
// ============================================================

typedef struct
{
    char      buffer[UART_RX_BUFFER_SIZE];
    uint16_t  length;
    uint8_t   lineReady;
    uint8_t   overflow;
} UartLineBuffer;

// ============================================================
// UART TX 상태
// ============================================================

typedef struct
{
    char         currentBuffer[UART_TX_BUFFER_SIZE];
    uint16_t     currentLength;
    uint8_t      busy;

    uint32_t     startTickMs;
    uint32_t     timeoutMs;

    MsgQueue     queue;
    MsgQueueItem queueStorage[UART_TX_QUEUE_SIZE];
} UartTxContext;

// ============================================================
// UART channel
// ============================================================

typedef struct
{
    UartRxPendingBuffer rxPending;
    UartLineBuffer      rxLine;
    UartTxContext       tx;

    uint8_t             errorFlag;
    UartErrorCode       errorCode;
} UartChannel;

// ============================================================
// UART port
// ============================================================

typedef struct
{
    uint32_t                     instance;
    lpuart_state_t              *driverState;
    const lpuart_user_config_t  *userConfig;

    uint8_t                      rxByte;
    UartChannel                  channel;
} UartPort;

// ============================================================
// Node 화면 상태
// ============================================================

typedef struct
{
    char inputLine[NODE_UART_INPUT_VIEW_SIZE];
    char taskLine[NODE_UART_TASK_VIEW_SIZE];
    char resultLine[NODE_UART_RESULT_VIEW_SIZE];
    uint8_t refreshRequired;
} NodeUartView;

// ============================================================
// Node context
// ============================================================

typedef struct
{
    UartPort         uart;
    NodeUartState    state;

    char             lineBuffer[UART_LINE_BUFFER_SIZE];
    uint16_t         lineLength;

    NodeUartView     view;

    CtrlMailbox     *ctrlMailbox;
    CtrlResultBox   *ctrlResultBox;

    uint8_t          nodeId;
    uint8_t          errorFlag;
} NodeUartContext;

#endif
