#ifndef SVC_COMM_ENDPOINT_H
#define SVC_COMM_ENDPOINT_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_status.h"
#include "core/emb_time.h"
#include "infra/emb_msg_queue.h"

typedef enum
{
    SVC_COMM_KIND_COMMAND = 0,
    SVC_COMM_KIND_RESPONSE,
    SVC_COMM_KIND_EVENT,
    SVC_COMM_KIND_TEXT
} svc_comm_kind_t;

typedef struct
{
    uint8_t kind;
    uint8_t target;
    uint16_t requestId;
    uint8_t payload[32];
    uint8_t payloadLength;
} svc_comm_message_t;

typedef struct
{
    uint8_t inUse;
    uint16_t requestId;
    emb_tick_t startTick;
    emb_tick_t timeoutTicks;
} svc_comm_pending_t;

typedef struct
{
    svc_comm_message_t *rxStorage;
    svc_comm_message_t *txStorage;
    size_t rxCapacity;
    size_t txCapacity;
    svc_comm_pending_t *pending;
    size_t pendingCount;
} svc_comm_endpoint_config_t;

typedef struct
{
    svc_comm_endpoint_config_t config;
    emb_fault_stats_t stats;
    emb_msg_queue_t rxQueue;
    emb_msg_queue_t txQueue;
    uint16_t nextRequestId;
} svc_comm_endpoint_t;

emb_status_t svc_comm_endpoint_init(svc_comm_endpoint_t *svc,
                                    const svc_comm_endpoint_config_t *config);
emb_status_t svc_comm_endpoint_push_rx(svc_comm_endpoint_t *svc, const svc_comm_message_t *msg);
emb_status_t svc_comm_endpoint_push_tx(svc_comm_endpoint_t *svc, const svc_comm_message_t *msg);
emb_status_t svc_comm_endpoint_pop_rx(svc_comm_endpoint_t *svc, svc_comm_message_t *outMsg);
emb_status_t svc_comm_endpoint_pop_tx(svc_comm_endpoint_t *svc, svc_comm_message_t *outMsg);
emb_status_t svc_comm_endpoint_allocate_request(svc_comm_endpoint_t *svc,
                                                emb_tick_t now,
                                                emb_tick_t timeout,
                                                uint16_t *outRequestId);
void svc_comm_endpoint_process_timeouts(svc_comm_endpoint_t *svc, emb_tick_t now);

#endif
