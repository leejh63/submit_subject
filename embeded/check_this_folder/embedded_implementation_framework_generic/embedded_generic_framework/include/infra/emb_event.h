#ifndef EMB_EVENT_H
#define EMB_EVENT_H

#include <stdint.h>

typedef enum
{
    EMB_EVENT_NONE = 0,
    EMB_EVENT_TICK,
    EMB_EVENT_RX_READY,
    EMB_EVENT_TX_DONE,
    EMB_EVENT_DMA_HALF,
    EMB_EVENT_DMA_FULL,
    EMB_EVENT_TIMEOUT,
    EMB_EVENT_ERROR,
    EMB_EVENT_APP_BASE = 100
} emb_event_id_t;

typedef struct
{
    uint16_t id;
    uint16_t source;
    uint32_t arg0;
    uint32_t arg1;
} emb_event_t;

#endif
