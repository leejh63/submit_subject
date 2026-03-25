#ifndef EMB_FAULT_H
#define EMB_FAULT_H

#include <stdint.h>
#include "emb_status.h"

typedef struct
{
    uint32_t okCount;
    uint32_t errorCount;
    uint32_t timeoutCount;
    uint32_t dropCount;
    uint32_t retryCount;
    emb_status_t lastError;
} emb_fault_stats_t;

static inline void emb_fault_record_ok(emb_fault_stats_t *stats)
{
    if (stats != 0)
        stats->okCount++;
}

static inline void emb_fault_record_error(emb_fault_stats_t *stats, emb_status_t error)
{
    if (stats == 0)
        return;

    stats->errorCount++;
    stats->lastError = error;
}

#endif
