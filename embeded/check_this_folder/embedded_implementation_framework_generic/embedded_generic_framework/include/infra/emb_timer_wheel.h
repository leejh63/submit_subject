#ifndef EMB_TIMER_WHEEL_H
#define EMB_TIMER_WHEEL_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"
#include "core/emb_time.h"

typedef void (*emb_timer_cb_t)(void *context);

typedef struct
{
    uint8_t active;
    emb_tick_t deadline;
    emb_timer_cb_t callback;
    void *context;
} emb_timer_slot_t;

typedef struct
{
    emb_timer_slot_t *slots;
    size_t capacity;
} emb_timer_wheel_t;

emb_status_t emb_timer_wheel_init(emb_timer_wheel_t *wheel,
                                  emb_timer_slot_t *slots,
                                  size_t capacity);
emb_status_t emb_timer_start(emb_timer_wheel_t *wheel,
                             size_t index,
                             emb_tick_t now,
                             emb_tick_t delay,
                             emb_timer_cb_t callback,
                             void *context);
emb_status_t emb_timer_cancel(emb_timer_wheel_t *wheel, size_t index);
void emb_timer_process(emb_timer_wheel_t *wheel, emb_tick_t now);

#endif
