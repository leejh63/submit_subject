// app_types.h
#ifndef APP_TYPES_H_
#define APP_TYPES_H_

#pragma once

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Common constants (protocol-level)
// ============================================================

#define DATA_SIZE                     (8U)

// CMD payload
#define MOTOR_TARGET_LEFT             (0U) // OPEN
#define MOTOR_TARGET_RIGHT            (1U) // CLOSE
#define MOTOR_TARGET_STUCK            (2U)

// STATUS payload
#define RANGE_OPEN                    (0U)
#define RANGE_STUCK                   (1U)
#define RANGE_CLOSE                   (2U)

// STATUS control (slave reports)
#define CTRL_BTN                      (0U)
#define CTRL_CMD                      (1U)
#define CTRL_POT                      (2U)

// ============================================================
// LIN frame direction
// ============================================================
typedef enum
{
    LIN_DIR_TX = 0,
    LIN_DIR_RX = 1
} lin_dir_t;

// ============================================================
// Frame definition (id/dir/buffer/len/timeout)
// ============================================================
typedef struct
{
    uint8_t    id;
    lin_dir_t  dir;
    uint8_t   *buf;
    uint8_t    len;
    uint16_t   timeoutCnt;
} lin_frame_def_t;

// ============================================================
// Request object (transaction)
// ============================================================
typedef enum
{
    REQ_SRC_BTN = 0,
    REQ_SRC_UART,
    REQ_SRC_PERIODIC
} req_source_t;

typedef enum
{
    REQ_TYPE_NONE = 0,
    REQ_TYPE_CMD,
    REQ_TYPE_STATUS
} req_type_t;

typedef struct
{
    req_type_t   type;
    req_source_t src;
    uint8_t      frameIndex;

    uint8_t      maxRetry;
    uint8_t      retryCount;
    uint32_t     deadlineTick; // 0이면 app deadline 미사용
} lin_req_t;

// ============================================================
// LIN Engine state
// ============================================================
typedef enum
{
    ENG_IDLE = 0,
    ENG_WAIT_PID,
    ENG_WAIT_TX,
    ENG_WAIT_RX
} lin_eng_state_t;

#endif /* APP_TYPES_H_ */
