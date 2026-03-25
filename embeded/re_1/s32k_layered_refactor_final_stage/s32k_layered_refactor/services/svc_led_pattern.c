#include "services/svc_led_pattern.h"

static void SvcLedPattern_SetSafe(SvcLedPattern *svc)
{
    (void)DrvLed_SetRedGreen(svc->config.led, 0U, 1U);
}

static void SvcLedPattern_SetWarning(SvcLedPattern *svc)
{
    (void)DrvLed_SetRedGreen(svc->config.led, 1U, 1U);
}

static void SvcLedPattern_SetDanger(SvcLedPattern *svc)
{
    (void)DrvLed_SetRedGreen(svc->config.led, 1U, 0U);
}

static void SvcLedPattern_SetEmergency(SvcLedPattern *svc, uint8_t on)
{
    (void)DrvLed_SetRedGreen(svc->config.led, on, 0U);
}

EmbResult SvcLedPattern_Init(SvcLedPattern *svc, const SvcLedPatternConfig *config)
{
    if (svc == 0 || config == 0 || config->led == 0)
        return EMB_EINVAL;

    svc->config = *config;
    svc->emergencyLatched = 0U;
    svc->blinkOn = 0U;
    svc->lastBlinkTick = 0U;
    svc->currentZone = SVC_ZONE_SAFE;
    return EMB_OK;
}

EmbResult SvcLedPattern_ApplyZone(SvcLedPattern *svc, SvcZone zone, emb_tick_t now)
{
    if (svc == 0)
        return EMB_EINVAL;

    svc->currentZone = zone;
    if (svc->emergencyLatched != 0U)
        return EMB_OK;

    switch (zone)
    {
        case SVC_ZONE_SAFE:
            SvcLedPattern_SetSafe(svc);
            break;
        case SVC_ZONE_WARNING:
            SvcLedPattern_SetWarning(svc);
            break;
        case SVC_ZONE_DANGER:
            SvcLedPattern_SetDanger(svc);
            break;
        case SVC_ZONE_EMERGENCY:
        default:
            svc->emergencyLatched = 1U;
            svc->blinkOn = 1U;
            svc->lastBlinkTick = now;
            SvcLedPattern_SetEmergency(svc, svc->blinkOn);
            break;
    }

    return EMB_OK;
}

EmbResult SvcLedPattern_ClearEmergency(SvcLedPattern *svc)
{
    if (svc == 0)
        return EMB_EINVAL;

    svc->emergencyLatched = 0U;
    svc->blinkOn = 0U;

    switch (svc->currentZone)
    {
        case SVC_ZONE_SAFE:
            SvcLedPattern_SetSafe(svc);
            break;
        case SVC_ZONE_WARNING:
            SvcLedPattern_SetWarning(svc);
            break;
        case SVC_ZONE_DANGER:
            SvcLedPattern_SetDanger(svc);
            break;
        case SVC_ZONE_EMERGENCY:
        default:
            SvcLedPattern_SetEmergency(svc, 1U);
            break;
    }

    return EMB_OK;
}

void SvcLedPattern_Process(SvcLedPattern *svc, emb_tick_t now)
{
    if (svc == 0 || svc->emergencyLatched == 0U)
        return;

    if (EmbTime_IsExpired(now, svc->lastBlinkTick, svc->config.blinkPeriod500us) == 0U)
        return;

    svc->lastBlinkTick = now;
    svc->blinkOn = (svc->blinkOn == 0U) ? 1U : 0U;
    SvcLedPattern_SetEmergency(svc, svc->blinkOn);
}

uint8_t SvcLedPattern_IsEmergencyLatched(const SvcLedPattern *svc)
{
    if (svc == 0)
        return 0U;

    return svc->emergencyLatched;
}
