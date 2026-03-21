#ifndef APP_STATECHART_H
#define APP_STATECHART_H

#include <stdint.h>
#include "infra/emb_state_machine.h"

typedef enum
{
    APP_STATE_BOOT = 0,
    APP_STATE_IDLE,
    APP_STATE_ACTIVE,
    APP_STATE_DEGRADED,
    APP_STATE_FAULT
} app_state_t;

void app_statechart_init(void *context);
void app_statechart_dispatch(const emb_event_t *event);

#endif
