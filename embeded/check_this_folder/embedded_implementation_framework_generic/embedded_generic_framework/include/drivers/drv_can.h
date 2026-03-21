#ifndef DRV_CAN_H
#define DRV_CAN_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_can.h"
#include "infra/emb_msg_queue.h"

typedef struct
{
    hal_can_t *hal;
    uint32_t bitrate;
    hal_can_frame_t *rxQueueStorage;
    size_t rxQueueCapacity;
} drv_can_config_t;

typedef struct
{
    drv_can_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
    emb_msg_queue_t rxQueue;
    volatile uint8_t rxPending;
} drv_can_t;

emb_status_t drv_can_init(drv_can_t *drv, const drv_can_config_t *config);
emb_status_t drv_can_start(drv_can_t *drv);
emb_status_t drv_can_send(drv_can_t *drv, const hal_can_frame_t *frame);
emb_status_t drv_can_receive(drv_can_t *drv, hal_can_frame_t *outFrame);
void drv_can_on_rx_irq(drv_can_t *drv, const hal_can_frame_t *frame);
void drv_can_process(drv_can_t *drv);

#endif
