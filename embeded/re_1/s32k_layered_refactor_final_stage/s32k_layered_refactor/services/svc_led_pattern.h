#ifndef SVC_LED_PATTERN_H
#define SVC_LED_PATTERN_H

#include <stdint.h>
#include "core/emb_result.h"
#include "drivers/drv_led.h"
#include "services/svc_zone.h"
#include "core/emb_time.h"

typedef struct
{
    DrvLed *led;
    uint32_t blinkPeriod500us;
} SvcLedPatternConfig;

typedef struct
{
    SvcLedPatternConfig config;
    uint8_t emergencyLatched;
    uint8_t blinkOn;
    emb_tick_t lastBlinkTick;
    SvcZone currentZone;
} SvcLedPattern;

EmbResult SvcLedPattern_Init(SvcLedPattern *svc, const SvcLedPatternConfig *config);
EmbResult SvcLedPattern_ApplyZone(SvcLedPattern *svc, SvcZone zone, emb_tick_t now);
EmbResult SvcLedPattern_ClearEmergency(SvcLedPattern *svc);
void SvcLedPattern_Process(SvcLedPattern *svc, emb_tick_t now);
uint8_t SvcLedPattern_IsEmergencyLatched(const SvcLedPattern *svc);

#endif
