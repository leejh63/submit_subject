#include "adc_hw.h"

#include <stddef.h>
#include <string.h>

#include "../platform/s32k_sdk/isosdk_adc.h"

static IsoSdkAdcContext s_adc_hw_context;

uint8_t AdcHw_IsSupported(void)
{
    return IsoSdk_AdcIsSupported();
}

InfraStatus AdcHw_Init(void *context)
{
    (void)context;
    (void)memset(&s_adc_hw_context, 0, sizeof(s_adc_hw_context));
    return (IsoSdk_AdcInit(&s_adc_hw_context) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

InfraStatus AdcHw_Sample(void *context, uint16_t *out_raw_value)
{
    (void)context;
    return (IsoSdk_AdcSample(&s_adc_hw_context, out_raw_value) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}
