#ifndef DRV_LIN_H
#define DRV_LIN_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_lin.h"

typedef enum
{
    DRV_LIN_ROLE_MASTER = 0,
    DRV_LIN_ROLE_SLAVE
} drv_lin_role_t;

typedef struct
{
    hal_lin_t *hal;
    uint32_t baudrate;
    drv_lin_role_t role;
} drv_lin_config_t;

typedef struct
{
    drv_lin_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
} drv_lin_t;

emb_status_t drv_lin_init(drv_lin_t *drv, const drv_lin_config_t *config);
emb_status_t drv_lin_send_header(drv_lin_t *drv, uint8_t pid);
emb_status_t drv_lin_send_response(drv_lin_t *drv, const uint8_t *data, size_t length);
emb_status_t drv_lin_read_response(drv_lin_t *drv, uint8_t *data, size_t length);
emb_status_t drv_lin_sleep(drv_lin_t *drv);
emb_status_t drv_lin_wakeup(drv_lin_t *drv);

#endif
