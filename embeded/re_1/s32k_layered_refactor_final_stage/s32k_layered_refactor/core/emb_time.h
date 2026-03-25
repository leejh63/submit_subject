#ifndef EMB_TIME_H
#define EMB_TIME_H

#include <stdint.h>

typedef uint32_t emb_tick_t;

static inline uint32_t EmbTime_Elapsed(emb_tick_t now, emb_tick_t start)
{
    return (uint32_t)(now - start);
}

static inline uint8_t EmbTime_IsExpired(emb_tick_t now,
                                        emb_tick_t start,
                                        emb_tick_t period)
{
    return (EmbTime_Elapsed(now, start) >= period) ? 1U : 0U;
}

#endif
