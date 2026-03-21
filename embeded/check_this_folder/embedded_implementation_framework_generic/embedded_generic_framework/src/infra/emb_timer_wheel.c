#include "infra/emb_timer_wheel.h"

emb_status_t emb_timer_wheel_init(emb_timer_wheel_t *wheel,
                                  emb_timer_slot_t *slots,
                                  size_t capacity)
{
    size_t i;

    if ((wheel == 0) || (slots == 0) || (capacity == 0U))
        return EMB_EINVAL;

    wheel->slots = slots;
    wheel->capacity = capacity;

    for (i = 0U; i < capacity; ++i)
        wheel->slots[i] = (emb_timer_slot_t){0};

    return EMB_OK;
}

emb_status_t emb_timer_start(emb_timer_wheel_t *wheel,
                             size_t index,
                             emb_tick_t now,
                             emb_tick_t delay,
                             emb_timer_cb_t callback,
                             void *context)
{
    if ((wheel == 0) || (index >= wheel->capacity) || (callback == 0))
        return EMB_EINVAL;

    wheel->slots[index].active = 1U;
    wheel->slots[index].deadline = now + delay;
    wheel->slots[index].callback = callback;
    wheel->slots[index].context = context;
    return EMB_OK;
}

emb_status_t emb_timer_cancel(emb_timer_wheel_t *wheel, size_t index)
{
    if ((wheel == 0) || (index >= wheel->capacity))
        return EMB_EINVAL;

    wheel->slots[index].active = 0U;
    return EMB_OK;
}

void emb_timer_process(emb_timer_wheel_t *wheel, emb_tick_t now)
{
    size_t i;

    if (wheel == 0)
        return;

    for (i = 0U; i < wheel->capacity; ++i)
    {
        if ((wheel->slots[i].active != 0U) &&
            emb_tick_expired(now, wheel->slots[i].deadline, 0U))
        {
            wheel->slots[i].active = 0U;
            if (wheel->slots[i].callback != 0)
                wheel->slots[i].callback(wheel->slots[i].context);
        }
    }
}
