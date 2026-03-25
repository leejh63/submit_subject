/*
 * LIN sensor slave 전용 애플리케이션 context 헤더다.
 * ADC, LED, LIN 상태 게시에 필요한 필드만 남겨,
 * console과 CAN 없이 센서 노드 흐름만 바로 읽을 수 있게 한다.
 */
#ifndef APP_CORE_H
#define APP_CORE_H

#include "../services/adc_module.h"
#include "../core/infra_types.h"
#include "../drivers/led_module.h"
#include "../services/lin_module.h"

typedef struct
{
    uint8_t   initialized;
    uint8_t   local_node_id;
    uint8_t   lin_enabled;
    uint8_t   adc_enabled;
    uint8_t   led2_enabled;
    uint32_t  heartbeat_count;
    LinModule lin_module;
    LedModule slave2_led;
    AdcModule adc_module;
    // define으로 상수값 빼기,
    char      adc_text[48];
    char      lin_input_text[48];
    char      lin_link_text[32];
} AppCore;

InfraStatus AppCore_Init(AppCore *app);
void        AppCore_OnTickIsr(void *context);
void        AppCore_TaskHeartbeat(AppCore *app, uint32_t now_ms);
void        AppCore_TaskLinFast(AppCore *app, uint32_t now_ms);
void        AppCore_TaskLed(AppCore *app, uint32_t now_ms);
void        AppCore_TaskAdc(AppCore *app, uint32_t now_ms);

#endif
