#ifndef SVC_ZONE_H
#define SVC_ZONE_H

#include <stdint.h>

typedef enum
{
    SVC_ZONE_SAFE = 0U,
    SVC_ZONE_WARNING,
    SVC_ZONE_DANGER,
    SVC_ZONE_EMERGENCY
} SvcZone;

typedef struct
{
    uint16_t rangeMax;
    uint16_t safeMax;
    uint16_t warningMax;
    uint16_t emergencyMin;
} SvcZoneConfig;

SvcZone SvcZone_Classify(const SvcZoneConfig *config, uint16_t raw);

#endif
