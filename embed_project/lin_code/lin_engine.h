// lin_engine.h

#ifndef LIN_ENGINE_H_
#define LIN_ENGINE_H_

#pragma once

#include "app_types.h"

#include <stdint.h>
#include <stdbool.h>

#define FRAME_MOTOR_CMD (1U) // Master -> Slave (TX)
#define FRAME_SLAVE_STATUS (2U) // Slave -> Master (RX)
#define TIMEOUT_LIN_CNT (500U) // LIN timeout counter

// comment?
typedef void (*lin_engine_done_cb_t)(const lin_req_t *req, bool ok);

// comment?
void lin_engine_init(lin_engine_done_cb_t done_cb, uint32_t nowTick);

// comment?
void lin_engine_install_callback(void);

// comment?
void lin_engine_step(uint32_t nowTick);

// comment?
void lin_engine_poll(uint32_t nowTick);

// comment?
bool            lin_engine_is_idle(void);
lin_eng_state_t lin_engine_state(void);
lin_req_t       lin_engine_active_req(void);
uint8_t         lin_engine_last_event(void);
uint8_t         lin_engine_err_code(void);

// comment?
void lin_engine_request_cmd(req_source_t src, uint8_t target);
void lin_engine_request_status(req_source_t src);

// comment?
const uint8_t *lin_engine_status_buf(void);

#endif /* LIN_ENGINE_H_ */
