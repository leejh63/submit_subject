#ifndef SVC_DEFERRED_WORK_H
#define SVC_DEFERRED_WORK_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_status.h"
#include "infra/emb_msg_queue.h"

typedef void (*svc_deferred_fn_t)(void *context);

typedef struct
{
    svc_deferred_fn_t fn;
    void *context;
} svc_deferred_item_t;

typedef struct
{
    svc_deferred_item_t *storage;
    size_t capacity;
} svc_deferred_work_config_t;

typedef struct
{
    svc_deferred_work_config_t config;
    emb_fault_stats_t stats;
    emb_msg_queue_t queue;
} svc_deferred_work_t;

emb_status_t svc_deferred_work_init(svc_deferred_work_t *svc,
                                    const svc_deferred_work_config_t *config);
emb_status_t svc_deferred_work_post(svc_deferred_work_t *svc,
                                    svc_deferred_fn_t fn,
                                    void *context);
void svc_deferred_work_process(svc_deferred_work_t *svc);

#endif
