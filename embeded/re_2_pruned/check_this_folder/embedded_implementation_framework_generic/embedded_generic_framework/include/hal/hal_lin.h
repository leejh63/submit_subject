#ifndef HAL_LIN_H
#define HAL_LIN_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_lin hal_lin_t;

typedef struct
{
    emb_status_t (*configure)(hal_lin_t *hal, uint32_t baudrate, uint8_t isMaster);
    emb_status_t (*send_header)(hal_lin_t *hal, uint8_t pid);
    emb_status_t (*send_response)(hal_lin_t *hal, const uint8_t *data, size_t length);
    emb_status_t (*read_response)(hal_lin_t *hal, uint8_t *data, size_t length);
    emb_status_t (*send_sleep)(hal_lin_t *hal);
    emb_status_t (*send_wakeup)(hal_lin_t *hal);
} hal_lin_ops_t;

struct hal_lin
{
    const hal_lin_ops_t *ops;
    void *context;
};

#endif
