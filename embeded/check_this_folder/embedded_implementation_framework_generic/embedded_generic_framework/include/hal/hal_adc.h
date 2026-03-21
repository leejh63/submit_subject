#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_adc hal_adc_t;

typedef struct
{
    emb_status_t (*configure_channel)(hal_adc_t *hal, uint32_t channel, uint32_t sampleTime);
    emb_status_t (*start_single)(hal_adc_t *hal, uint32_t channel);
    emb_status_t (*start_scan_dma)(hal_adc_t *hal, const uint32_t *channels, size_t count, uint16_t *buffer);
    emb_status_t (*stop)(hal_adc_t *hal);
    emb_status_t (*calibrate)(hal_adc_t *hal);
} hal_adc_ops_t;

struct hal_adc
{
    const hal_adc_ops_t *ops;
    void *context;
};

#endif
