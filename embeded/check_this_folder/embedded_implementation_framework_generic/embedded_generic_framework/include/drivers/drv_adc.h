#ifndef DRV_ADC_H
#define DRV_ADC_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_module.h"
#include "core/emb_status.h"
#include "hal/hal_adc.h"

typedef void (*drv_adc_scan_done_cb_t)(void *context);

typedef struct
{
    hal_adc_t *hal;
    const uint32_t *channels;
    size_t channelCount;
    uint16_t *sampleBuffer;
    uint32_t sampleTime;
    drv_adc_scan_done_cb_t scanDoneCb;
    void *context;
} drv_adc_config_t;

typedef struct
{
    drv_adc_config_t config;
    emb_module_state_t state;
    emb_fault_stats_t stats;
    volatile uint8_t scanPending;
} drv_adc_t;

emb_status_t drv_adc_init(drv_adc_t *drv, const drv_adc_config_t *config);
emb_status_t drv_adc_calibrate(drv_adc_t *drv);
emb_status_t drv_adc_start_scan_dma(drv_adc_t *drv);
void drv_adc_on_dma_complete_irq(drv_adc_t *drv);
void drv_adc_process(drv_adc_t *drv);

#endif
