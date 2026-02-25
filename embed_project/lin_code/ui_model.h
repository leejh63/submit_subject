// ui_model.h

#ifndef UI_MODEL_H_
#define UI_MODEL_H_


#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "app_types.h"

typedef struct
{
    // UART/UI message
    char     msg[48];
    uint32_t tick500us;

    // LIN comm
    lin_eng_state_t eng_state;
    lin_req_t       active_req;
    uint8_t         last_event;
    uint8_t         err_code;

    // Slave status
    bool     slave_valid;
    uint8_t  pos;
    uint8_t  range;
    uint8_t  ctrl;
    uint8_t  err;

    // last ok/fail ticks
    uint32_t last_ok_tick;
    uint32_t last_fail_tick;

} ui_model_t;

void ui_model_init(ui_model_t *m);
void ui_model_set_msg(ui_model_t *m, const char *s);

#endif /* UI_MODEL_H_ */
