#include "services/svc_sensor_pipeline.h"

emb_status_t svc_sensor_pipeline_init(svc_sensor_pipeline_t *svc,
                                      const svc_sensor_pipeline_config_t *config)
{
    if ((svc == 0) || (config == 0) || (config->engineeringScaleDen == 0))
        return EMB_EINVAL;

    svc->config = *config;
    svc->stats = (emb_fault_stats_t){0};
    svc->filterAcc = 0;
    svc->lastSample = (svc_sensor_sample_t){0};
    return EMB_OK;
}

emb_status_t svc_sensor_pipeline_process_raw(svc_sensor_pipeline_t *svc,
                                             uint16_t raw,
                                             svc_sensor_sample_t *outSample)
{
    int32_t filtered;
    int32_t engineering;
    uint8_t valid = 1U;

    if ((svc == 0) || (outSample == 0))
        return EMB_EINVAL;

    if ((raw < svc->config.plausibilityMin) || (raw > svc->config.plausibilityMax))
        valid = 0U;

    svc->filterAcc += ((int32_t)raw - svc->filterAcc) >> svc->config.filterShift;
    filtered = svc->filterAcc;
    engineering = (filtered * svc->config.engineeringScaleNum) / svc->config.engineeringScaleDen;

    *outSample = (svc_sensor_sample_t){
        .raw = raw,
        .filtered = filtered,
        .engineering = engineering,
        .valid = valid
    };

    svc->lastSample = *outSample;
    return valid ? EMB_OK : EMB_ESTATE;
}
