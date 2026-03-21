#ifndef HAL_S32K_ADC_H
#define HAL_S32K_ADC_H

#include <stdint.h>
#include "core/emb_result.h"

typedef struct
{
    uint32_t instance;
    uint8_t group;
} HalS32kAdcChannel;

EmbResult HalS32kAdc_InitDefault(void);
EmbResult HalS32kAdc_ReadBlocking(HalS32kAdcChannel channel, uint16_t *outRaw);

#endif
