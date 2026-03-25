#include "app/app_statechart.h"

static emb_state_machine_t g_appSm;

static void app_on_boot(emb_state_machine_t *sm, const emb_event_t *event)
{
    (void)event;
    emb_state_machine_transition(sm, APP_STATE_IDLE);
}

static void app_on_idle(emb_state_machine_t *sm, const emb_event_t *event)
{
    if (event->id == EMB_EVENT_APP_BASE)
        emb_state_machine_transition(sm, APP_STATE_ACTIVE);
}

static void app_on_active(emb_state_machine_t *sm, const emb_event_t *event)
{
    if (event->id == EMB_EVENT_ERROR)
        emb_state_machine_transition(sm, APP_STATE_DEGRADED);
}

static void app_on_degraded(emb_state_machine_t *sm, const emb_event_t *event)
{
    if (event->id == EMB_EVENT_TIMEOUT)
        emb_state_machine_transition(sm, APP_STATE_FAULT);
}

static void app_on_fault(emb_state_machine_t *sm, const emb_event_t *event)
{
    (void)sm;
    (void)event;
}

static const emb_state_desc_t g_appStates[] =
{
    { APP_STATE_BOOT,  app_on_boot,     app_on_boot,     0 },
    { APP_STATE_IDLE,  0,               app_on_idle,     0 },
    { APP_STATE_ACTIVE,0,               app_on_active,   0 },
    { APP_STATE_DEGRADED,0,             app_on_degraded, 0 },
    { APP_STATE_FAULT, 0,               app_on_fault,    0 }
};

void app_statechart_init(void *context)
{
    emb_state_machine_init(&g_appSm,
                           g_appStates,
                           (uint16_t)(sizeof(g_appStates) / sizeof(g_appStates[0])),
                           APP_STATE_BOOT,
                           context);
}

void app_statechart_dispatch(const emb_event_t *event)
{
    emb_state_machine_dispatch(&g_appSm, event);
}
