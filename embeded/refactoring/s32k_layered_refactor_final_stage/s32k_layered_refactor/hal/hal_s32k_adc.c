#include "hal/hal_s32k_adc.h"
#include "platform/s32k_bindings.h"

EmbResult HalS32kAdc_InitDefault(void)
{
#if (S32K_LIN_SENSOR_ADC_ENABLE != 0U)
    (void)ADC_DRV_ConfigConverter(S32K_LIN_SENSOR_ADC_INSTANCE,
                                  &S32K_LIN_SENSOR_ADC_CONV_CONFIG);
    (void)ADC_DRV_AutoCalibration(S32K_LIN_SENSOR_ADC_INSTANCE);
    return EMB_OK;
#else
    return EMB_EUNSUPPORTED;
#endif
}

EmbResult HalS32kAdc_ReadBlocking(HalS32kAdcChannel channel, uint16_t *outRaw)
{
#if (S32K_LIN_SENSOR_ADC_ENABLE != 0U)
    uint16_t raw = 0U;

    if (outRaw == 0)
        return EMB_EINVAL;

    (void)ADC_DRV_ConfigChan(channel.instance,
                             channel.group,
                             &S32K_LIN_SENSOR_ADC_CH_CONFIG);
    (void)ADC_DRV_WaitConvDone(channel.instance);
    (void)ADC_DRV_GetChanResult(channel.instance, channel.group, &raw);

    if (raw > 4095U)
        raw = 4095U;

    *outRaw = raw;
    return EMB_OK;
#else
    (void)channel;
    (void)outRaw;
    return EMB_EUNSUPPORTED;
#endif
}
