#include "isosdk_adc.h"

#include <stddef.h>
#include <string.h>

#include "isosdk_sdk_bindings.h"

#ifdef ISOSDK_SDK_HAS_ADC

#define ISOSDK_ADC_GROUP  0U

static adc_chan_config_t s_iso_sdk_adc_chan_config;

uint8_t IsoSdk_AdcIsSupported(void)
{
    return 1U;
}

uint8_t IsoSdk_AdcInit(IsoSdkAdcContext *context)
{
    if (context == NULL)
    {
        return 0U;
    }

    (void)memset(context, 0, sizeof(*context));
    s_iso_sdk_adc_chan_config = ISOSDK_SDK_ADC_CHANNEL_CONFIG;
    s_iso_sdk_adc_chan_config.interruptEnable = false;

    ADC_DRV_ConfigConverter(ISOSDK_SDK_ADC_INSTANCE, &ISOSDK_SDK_ADC_CONVERTER_CONFIG);
    ADC_DRV_AutoCalibration(ISOSDK_SDK_ADC_INSTANCE);
    context->initialized = 1U;
    return 1U;
}

uint8_t IsoSdk_AdcSample(IsoSdkAdcContext *context, uint16_t *out_raw_value)
{
    uint16_t raw_value;

    if ((context == NULL) || (context->initialized == 0U) || (out_raw_value == NULL))
    {
        return 0U;
    }

    raw_value = 0U;
    ADC_DRV_ConfigChan(ISOSDK_SDK_ADC_INSTANCE, ISOSDK_ADC_GROUP, &s_iso_sdk_adc_chan_config);
    ADC_DRV_WaitConvDone(ISOSDK_SDK_ADC_INSTANCE);
    ADC_DRV_GetChanResult(ISOSDK_SDK_ADC_INSTANCE, ISOSDK_ADC_GROUP, &raw_value);

    if (raw_value > 4095U)
    {
        raw_value = 4095U;
    }

    *out_raw_value = raw_value;
    return 1U;
}

#else

uint8_t IsoSdk_AdcIsSupported(void)
{
    return 0U;
}

uint8_t IsoSdk_AdcInit(IsoSdkAdcContext *context)
{
    (void)context;
    return 0U;
}

uint8_t IsoSdk_AdcSample(IsoSdkAdcContext *context, uint16_t *out_raw_value)
{
    (void)context;

    if (out_raw_value != NULL)
    {
        *out_raw_value = 0U;
    }

    return 0U;
}

#endif
