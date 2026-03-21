#include <string.h>
#include "services/svc_comm_endpoint.h"

emb_status_t svc_comm_endpoint_init(svc_comm_endpoint_t *svc,
                                    const svc_comm_endpoint_config_t *config)
{
    emb_status_t status;

    if ((svc == 0) || (config == 0))
        return EMB_EINVAL;

    svc->config = *config;
    svc->stats = (emb_fault_stats_t){0};
    svc->nextRequestId = 1U;

    status = emb_msg_queue_init(&svc->rxQueue,
                                config->rxStorage,
                                sizeof(svc_comm_message_t),
                                config->rxCapacity);
    if (status != EMB_OK)
        return status;

    status = emb_msg_queue_init(&svc->txQueue,
                                config->txStorage,
                                sizeof(svc_comm_message_t),
                                config->txCapacity);
    if (status != EMB_OK)
        return status;

    memset(config->pending, 0, sizeof(config->pending[0]) * config->pendingCount);
    return EMB_OK;
}

emb_status_t svc_comm_endpoint_push_rx(svc_comm_endpoint_t *svc, const svc_comm_message_t *msg)
{
    if ((svc == 0) || (msg == 0))
        return EMB_EINVAL;

    return emb_msg_queue_push(&svc->rxQueue, msg);
}

emb_status_t svc_comm_endpoint_push_tx(svc_comm_endpoint_t *svc, const svc_comm_message_t *msg)
{
    if ((svc == 0) || (msg == 0))
        return EMB_EINVAL;

    return emb_msg_queue_push(&svc->txQueue, msg);
}

emb_status_t svc_comm_endpoint_pop_rx(svc_comm_endpoint_t *svc, svc_comm_message_t *outMsg)
{
    if ((svc == 0) || (outMsg == 0))
        return EMB_EINVAL;

    return emb_msg_queue_pop(&svc->rxQueue, outMsg);
}

emb_status_t svc_comm_endpoint_pop_tx(svc_comm_endpoint_t *svc, svc_comm_message_t *outMsg)
{
    if ((svc == 0) || (outMsg == 0))
        return EMB_EINVAL;

    return emb_msg_queue_pop(&svc->txQueue, outMsg);
}

emb_status_t svc_comm_endpoint_allocate_request(svc_comm_endpoint_t *svc,
                                                emb_tick_t now,
                                                emb_tick_t timeout,
                                                uint16_t *outRequestId)
{
    size_t i;

    if ((svc == 0) || (outRequestId == 0))
        return EMB_EINVAL;

    for (i = 0U; i < svc->config.pendingCount; ++i)
    {
        if (svc->config.pending[i].inUse == 0U)
        {
            svc->config.pending[i].inUse = 1U;
            svc->config.pending[i].requestId = svc->nextRequestId++;
            svc->config.pending[i].startTick = now;
            svc->config.pending[i].timeoutTicks = timeout;
            *outRequestId = svc->config.pending[i].requestId;
            return EMB_OK;
        }
    }

    return EMB_ENOSPACE;
}

void svc_comm_endpoint_process_timeouts(svc_comm_endpoint_t *svc, emb_tick_t now)
{
    size_t i;

    if (svc == 0)
        return;

    for (i = 0U; i < svc->config.pendingCount; ++i)
    {
        if ((svc->config.pending[i].inUse != 0U) &&
            emb_tick_expired(now,
                             svc->config.pending[i].startTick,
                             svc->config.pending[i].timeoutTicks))
        {
            svc->config.pending[i].inUse = 0U;
            svc->stats.timeoutCount++;
        }
    }
}
