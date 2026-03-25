#ifndef EMB_TIME_H
#define EMB_TIME_H

#include <stdint.h>

typedef uint32_t emb_tick_t;

static inline emb_tick_t emb_tick_elapsed(emb_tick_t now, emb_tick_t start)
{
    return (emb_tick_t)(now - start);
}

static inline uint8_t emb_tick_expired(emb_tick_t now, emb_tick_t start, emb_tick_t timeout)
{
    return (emb_tick_elapsed(now, start) >= timeout) ? 1U : 0U;
}

#endif
