/*
 * service 계층이 공유하는 UART 전송 자료구조 모음이다.
 * 이 헤더의 타입들은 RX 버퍼링과 TX queueing,
 * 그리고 콘솔 모듈이 들고 있는 전체 service 상태를 표현한다.
 */
#ifndef UART_TYPES_H
#define UART_TYPES_H

#include "infra/infra_queue.h"

#define UART_RX_LINE_SIZE            64U
#define UART_RX_PENDING_SIZE         32U
#define UART_TX_CHUNK_SIZE           128U
#define UART_TX_QUEUE_SIZE           8U
#define UART_DEFAULT_TIMEOUT_MS      100U

typedef enum
{
    UART_ERROR_NONE = 0,
    UART_ERROR_HW_INIT,
    UART_ERROR_RX_DRIVER,
    UART_ERROR_RX_PENDING_OVERFLOW,
    UART_ERROR_RX_LINE_OVERFLOW,
    UART_ERROR_TX_QUEUE_FULL,
    UART_ERROR_TX_DRIVER,
    UART_ERROR_TX_TIMEOUT
} UartErrorCode;

/*
 * UART callback이 받은 바이트를 저장하는 ring buffer다.
 * 저수준 driver는 바이트를 빠르게 여기에 밀어 넣고,
 * line 파싱은 나중에 일반 task context에서 수행하게 된다.
 */
typedef struct
{
    uint8_t           buffer[UART_RX_PENDING_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint8_t  overflow;
    volatile uint32_t overflow_count;
} UartRxPendingRing;

/*
 * 콘솔 입력을 위해 현재 조립 중인 line buffer다.
 * UartService는 printable 문자를 여기에 붙여 나가다가,
 * 완전한 line 또는 overflow를 감지하면 처리를 넘긴다.
 */
typedef struct
{
    char     buffer[UART_RX_LINE_SIZE];
    uint16_t length;
    uint8_t  line_ready;
    uint8_t  overflow;
} UartLineBuffer;

typedef struct
{
    uint8_t  data[UART_TX_CHUNK_SIZE];
    uint16_t length;
} UartTxChunk;

/*
 * UART service가 관리하는 buffered transmit 상태다.
 * 긴 텍스트 출력은 chunk로 잘라 두어,
 * console이 app을 막지 않고 여러 render 갱신을 queue에 올릴 수 있게 한다.
 */
typedef struct
{
    char       current_buffer[UART_TX_CHUNK_SIZE + 1U];
    uint16_t   current_length;
    uint8_t    busy;
    uint32_t   start_ms;
    uint32_t   timeout_ms;
    InfraQueue queue;
    UartTxChunk queue_storage[UART_TX_QUEUE_SIZE];
} UartTxContext;

/*
 * 콘솔 계층이 소유하는 전체 UART service 상태다.
 * 저수준 RX/TX 상태와 오류 추적,
 * 그리고 operator 인터페이스가 쓰는 현재 line buffer를 함께 가진다.
 */
typedef struct
{
    uint8_t           initialized;
    uint32_t          instance;
    uint8_t           rx_byte;
    UartRxPendingRing rx_pending;
    UartLineBuffer    rx_line;
    UartTxContext     tx;
    uint8_t           error_flag;
    uint32_t          error_count;
    UartErrorCode     error_code;
} UartService;

#endif
