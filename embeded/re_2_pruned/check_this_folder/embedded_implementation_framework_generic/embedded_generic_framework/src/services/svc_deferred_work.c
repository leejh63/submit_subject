#include "services/svc_deferred_work.h"

emb_status_t svc_deferred_work_init(svc_deferred_work_t *svc,
                                    const svc_deferred_work_config_t *config)
{
    if ((svc == 0) || (config == 0))
        return EMB_EINVAL;

    svc->config = *config;
    svc->stats = (emb_fault_stats_t){0};

    return emb_msg_queue_init(&svc->queue,
                              config->storage,
                              sizeof(svc_deferred_item_t),
                              config->capacity);
}

emb_status_t svc_deferred_work_post(svc_deferred_work_t *svc,
                                    svc_deferred_fn_t fn,
                                    void *context)
{
    svc_deferred_item_t item;

    if ((svc == 0) || (fn == 0))
        return EMB_EINVAL;

    item.fn = fn;
    item.context = context;
    return emb_msg_queue_push(&svc->queue, &item);
}

void svc_deferred_work_process(svc_deferred_work_t *svc)
{
    svc_deferred_item_t item;

    if (svc == 0)
        return;

    while (emb_msg_queue_pop(&svc->queue, &item) == EMB_OK)
    {
        item.fn(item.context);
    }
}
