#include "infra/emb_state_machine.h"

static const emb_state_desc_t *emb_state_desc_find(const emb_state_machine_t *sm, uint16_t state)
{
    uint16_t i;

    for (i = 0U; i < sm->stateCount; ++i)
    {
        if (sm->table[i].state == state)
            return &sm->table[i];
    }

    return 0;
}

void emb_state_machine_init(emb_state_machine_t *sm,
                            const emb_state_desc_t *table,
                            uint16_t stateCount,
                            uint16_t initialState,
                            void *context)
{
    sm->table = table;
    sm->stateCount = stateCount;
    sm->currentState = initialState;
    sm->context = context;
}

void emb_state_machine_dispatch(emb_state_machine_t *sm, const emb_event_t *event)
{
    const emb_state_desc_t *desc;

    if ((sm == 0) || (event == 0))
        return;

    desc = emb_state_desc_find(sm, sm->currentState);
    if ((desc != 0) && (desc->onEvent != 0))
        desc->onEvent(sm, event);
}

void emb_state_machine_transition(emb_state_machine_t *sm, uint16_t newState)
{
    const emb_state_desc_t *oldDesc;
    const emb_state_desc_t *newDesc;
    emb_event_t transitionEvent = { EMB_EVENT_NONE, 0U, 0U, 0U };

    if (sm == 0)
        return;
    if (sm->currentState == newState)
        return;

    oldDesc = emb_state_desc_find(sm, sm->currentState);
    newDesc = emb_state_desc_find(sm, newState);
    if (newDesc == 0)
        return;

    if ((oldDesc != 0) && (oldDesc->onExit != 0))
        oldDesc->onExit(sm, &transitionEvent);

    sm->currentState = newState;

    if (newDesc->onEntry != 0)
        newDesc->onEntry(sm, &transitionEvent);
}
