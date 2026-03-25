#ifndef DRV_LED_H
#define DRV_LED_H

#include <stdint.h>
#include "core/emb_result.h"

typedef struct
{
    void *gpioBase;
    uint32_t redPin;
    uint32_t greenPin;
    uint32_t pinMask;
    uint8_t activeLevel;
    uint8_t inactiveLevel;
} DrvLedConfig;

typedef struct
{
    DrvLedConfig config;
    uint8_t started;
} DrvLed;

EmbResult DrvLed_Init(DrvLed *led, const DrvLedConfig *config);
EmbResult DrvLed_Start(DrvLed *led);
EmbResult DrvLed_SetRedGreen(DrvLed *led, uint8_t redOn, uint8_t greenOn);

#endif
