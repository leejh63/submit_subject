#ifndef HAL_CAN_H
#define HAL_CAN_H

#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_can hal_can_t;

typedef struct
{
    uint32_t id;
    uint8_t dlc;
    uint8_t data[64];
    uint8_t isExtended;
    uint8_t isFd;
} hal_can_frame_t;

typedef struct
{
    emb_status_t (*configure)(hal_can_t *hal, uint32_t bitrate);
    emb_status_t (*start)(hal_can_t *hal);
    emb_status_t (*send)(hal_can_t *hal, const hal_can_frame_t *frame);
    emb_status_t (*set_filter)(hal_can_t *hal, uint32_t filterId, uint32_t mask, uint8_t extended);
    emb_status_t (*recover_bus_off)(hal_can_t *hal);
} hal_can_ops_t;

struct hal_can
{
    const hal_can_ops_t *ops;
    void *context;
};

#endif
