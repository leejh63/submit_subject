#include "drivers/drv_adc.h"

EmbResult DrvAdc_Init(DrvAdc *adc, const DrvAdcConfig *config)
{
    if (adc == 0 || config == 0)
        return EMB_EINVAL;

    adc->config = *config;
    adc->lastRaw = 0U;
    adc->started = 0U;
    return EMB_OK;
}

EmbResult DrvAdc_Start(DrvAdc *adc)
{
    if (adc == 0)
        return EMB_EINVAL;

    if (HalS32kAdc_InitDefault() != EMB_OK)
        return EMB_EIO;

    adc->started = 1U;
    return EMB_OK;
}

EmbResult DrvAdc_Read(DrvAdc *adc, uint16_t *outRaw)
{
    if (adc == 0 || outRaw == 0)
        return EMB_EINVAL;
    if (adc->started == 0U)
        return EMB_ESTATE;

    if (HalS32kAdc_ReadBlocking(adc->config.channel, outRaw) != EMB_OK)
        return EMB_EIO;

    adc->lastRaw = *outRaw;
    return EMB_OK;
}
