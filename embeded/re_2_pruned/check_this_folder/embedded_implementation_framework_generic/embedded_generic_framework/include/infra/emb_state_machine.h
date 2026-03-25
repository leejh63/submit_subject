#ifndef EMB_STATE_MACHINE_H
#define EMB_STATE_MACHINE_H

#include <stdint.h>
#include "infra/emb_event.h"

typedef struct emb_state_machine emb_state_machine_t;

typedef void (*emb_state_action_t)(emb_state_machine_t *sm, const emb_event_t *event);

typedef struct
{
    uint16_t state;
    emb_state_action_t onEntry;
    emb_state_action_t onEvent;
    emb_state_action_t onExit;
} emb_state_desc_t;

struct emb_state_machine
{
    uint16_t currentState;
    const emb_state_desc_t *table;
    uint16_t stateCount;
    void *context;
};

void emb_state_machine_init(emb_state_machine_t *sm,
                            const emb_state_desc_t *table,
                            uint16_t stateCount,
                            uint16_t initialState,
                            void *context);
void emb_state_machine_dispatch(emb_state_machine_t *sm, const emb_event_t *event);
void emb_state_machine_transition(emb_state_machine_t *sm, uint16_t newState);

#endif
