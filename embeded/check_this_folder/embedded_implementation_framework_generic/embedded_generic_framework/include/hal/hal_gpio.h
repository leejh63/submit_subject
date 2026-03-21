#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_gpio hal_gpio_t;

typedef struct
{
    emb_status_t (*configure_output)(hal_gpio_t *hal, uint32_t pin, uint8_t activeHigh, uint8_t initialLevel);
    emb_status_t (*configure_input)(hal_gpio_t *hal, uint32_t pin, uint8_t pullMode);
    emb_status_t (*write)(hal_gpio_t *hal, uint32_t pin, uint8_t level);
    emb_status_t (*read)(hal_gpio_t *hal, uint32_t pin, uint8_t *level);
} hal_gpio_ops_t;

struct hal_gpio
{
    const hal_gpio_ops_t *ops;
    void *context;
};

#endif
