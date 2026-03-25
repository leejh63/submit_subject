/*
 * ADC 샘플링과 zone 분류 구현부다.
 * 이 모듈은 raw sample을 해석된 상태로 바꾸고,
 * 승인 절차에서 사용하는 emergency latch를 유지한다.
 */
#include "adc_module.h"

#include <stddef.h>
#include <string.h>

/*
 * raw ADC sample 하나를 semantic zone으로 매핑한다.
 * threshold 비교 순서를 여기 모아두어,
 * task 경로는 sample과 latch 유지에 집중할 수 있게 한다.
 */
static uint8_t AdcModule_ClassifyZone(const AdcConfig *config, uint16_t raw_value)
{
    if (raw_value < config->safe_max)
    {
        return ADC_ZONE_SAFE;
    }

    if (raw_value < config->warning_max)
    {
        return ADC_ZONE_WARNING;
    }

    if (raw_value < config->emergency_min)
    {
        return ADC_ZONE_DANGER;
    }

    return ADC_ZONE_EMERGENCY;
}

/*
 * 보드 callback과 threshold로 ADC 모듈을 초기화한다.
 * 먼저 설정을 검증한 뒤,
 * 필요한 하드웨어 준비를 platform binding에 요청한다.
 */
InfraStatus AdcModule_Init(AdcModule *module, const AdcConfig *config)
{
    InfraStatus status;

    if ((module == NULL) || (config == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(module, 0, sizeof(*module));
    module->config = *config;

    if ((module->config.sample_fn == NULL) || (module->config.range_max == 0U))
    {
        return INFRA_STATUS_NOT_READY;
    }

    if (module->config.init_fn != NULL)
    {
        status = module->config.init_fn(module->config.hw_context);
        if (status != INFRA_STATUS_OK)
        {
            return status;
        }
    }

    module->initialized = 1U;
    return INFRA_STATUS_OK;
}

/*
 * 주기적으로 ADC를 샘플링하고 현재 snapshot을 갱신한다.
 * emergency latch 동작을 여기서 강제하여,
 * 상위 계층이 센서 상태를 항상 같은 방식으로 해석하도록 만든다.
 */
void AdcModule_Task(AdcModule *module, uint32_t now_ms)
{
    InfraStatus status;
    uint16_t    raw_value;
    uint16_t    range_ceiling;

    if ((module == NULL) || (module->initialized == 0U))
    {
        return;
    }

    if (Infra_TimeIsDue(now_ms, module->last_sample_ms, module->config.sample_period_ms) == 0U)
    {
        return;
    }

    module->last_sample_ms = now_ms;
    raw_value = 0U;
    status = module->config.sample_fn(module->config.hw_context, &raw_value);
    if (status != INFRA_STATUS_OK)
    {
        module->snapshot.sample_error = 1U;
        return;
    }

    range_ceiling = (module->config.range_max > 0U) ? (uint16_t)(module->config.range_max - 1U) : 0U;
    if (raw_value > range_ceiling)
    {
        raw_value = range_ceiling;
    }

    module->snapshot.raw_value = raw_value;
    module->snapshot.zone = AdcModule_ClassifyZone(&module->config, raw_value);
    module->snapshot.has_sample = 1U;
    module->snapshot.sample_error = 0U;

    if (module->snapshot.zone == ADC_ZONE_EMERGENCY)
    {
        module->snapshot.emergency_latched = 1U;
    }
}

InfraStatus AdcModule_GetSnapshot(const AdcModule *module, AdcSnapshot *out_snapshot)
{
    if ((module == NULL) || (out_snapshot == NULL) || (module->initialized == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    *out_snapshot = module->snapshot;
    return module->snapshot.has_sample != 0U ? INFRA_STATUS_OK : INFRA_STATUS_EMPTY;
}

/*
 * 승인 이후 emergency latch를 지우려고 시도한다.
 * 최신 zone이 더 이상 emergency가 아닐 때만 latch를 지워,
 * 너무 이른 복구를 막는다.
 */
InfraStatus AdcModule_ClearEmergencyLatch(AdcModule *module)
{
    if ((module == NULL) || (module->initialized == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (module->snapshot.has_sample == 0U)
    {
        return INFRA_STATUS_EMPTY;
    }

    if (module->snapshot.zone == ADC_ZONE_EMERGENCY)
    {
        return INFRA_STATUS_BUSY;
    }

    module->snapshot.emergency_latched = 0U;
    return INFRA_STATUS_OK;
}

const char *AdcModule_ZoneText(uint8_t zone)
{
    switch (zone)
    {
        case ADC_ZONE_SAFE:
            return "safe";

        case ADC_ZONE_WARNING:
            return "warning";

        case ADC_ZONE_DANGER:
            return "danger";

        case ADC_ZONE_EMERGENCY:
            return "emergency";

        default:
            return "unknown";
    }
}
