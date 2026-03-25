#ifndef SVC_SENSOR_PIPELINE_H
#define SVC_SENSOR_PIPELINE_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_fault.h"
#include "core/emb_status.h"

typedef struct
{
    uint16_t raw;
    int32_t filtered;
    int32_t engineering;
    uint8_t valid;
} svc_sensor_sample_t;

typedef struct
{
    uint16_t plausibilityMin;
    uint16_t plausibilityMax;
    uint8_t filterShift;
    int32_t engineeringScaleNum;
    int32_t engineeringScaleDen;
} svc_sensor_pipeline_config_t;

typedef struct
{
    svc_sensor_pipeline_config_t config;
    emb_fault_stats_t stats;
    int32_t filterAcc;
    svc_sensor_sample_t lastSample;
} svc_sensor_pipeline_t;

emb_status_t svc_sensor_pipeline_init(svc_sensor_pipeline_t *svc,
                                      const svc_sensor_pipeline_config_t *config);
emb_status_t svc_sensor_pipeline_process_raw(svc_sensor_pipeline_t *svc,
                                             uint16_t raw,
                                             svc_sensor_sample_t *outSample);

#endif
