#ifndef HAL_S32K_GPIO_H
#define HAL_S32K_GPIO_H

#include <stdint.h>
#include "core/emb_result.h"

EmbResult HalS32kGpio_SetDirection(void *gpioBase, uint32_t pinMask);
EmbResult HalS32kGpio_WritePin(void *gpioBase, uint32_t pin, uint8_t value);
EmbResult HalS32kGpio_SetPins(void *gpioBase, uint32_t pinMask);

#endif
