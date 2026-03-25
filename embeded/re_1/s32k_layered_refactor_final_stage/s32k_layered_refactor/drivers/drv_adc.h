#ifndef DRV_ADC_H
#define DRV_ADC_H

#include <stdint.h>
#include "core/emb_result.h"
#include "hal/hal_s32k_adc.h"

typedef struct
{
    HalS32kAdcChannel channel;
} DrvAdcConfig;

typedef struct
{
    DrvAdcConfig config;
    uint16_t lastRaw;
    uint8_t started;
} DrvAdc;

EmbResult DrvAdc_Init(DrvAdc *adc, const DrvAdcConfig *config);
EmbResult DrvAdc_Start(DrvAdc *adc);
EmbResult DrvAdc_Read(DrvAdc *adc, uint16_t *outRaw);

#endif
