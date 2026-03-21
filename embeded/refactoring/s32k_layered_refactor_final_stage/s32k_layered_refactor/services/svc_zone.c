#include "services/svc_zone.h"

SvcZone SvcZone_Classify(const SvcZoneConfig *config, uint16_t raw)
{
    if (config == 0)
        return SVC_ZONE_SAFE;

    if (raw < config->safeMax)
        return SVC_ZONE_SAFE;
    if (raw < config->warningMax)
        return SVC_ZONE_WARNING;
    if (raw < config->emergencyMin)
        return SVC_ZONE_DANGER;

    return SVC_ZONE_EMERGENCY;
}
