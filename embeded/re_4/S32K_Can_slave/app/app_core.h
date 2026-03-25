/*
 * CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 context 헤더다.
 * slave1이 실제로 쓰는 CAN 상태, 버튼 debounce 상태,
 * 로컬 LED 제어기만 남겨 UART/콘솔 없이도 흐름이 보이게 한다.
 */
#ifndef APP_CORE_H
#define APP_CORE_H

#include "../services/can_module.h"
#include "../core/infra_types.h"
#include "../drivers/led_module.h"

typedef struct
{
    uint8_t    initialized;
    uint8_t    local_node_id;
    uint8_t    can_enabled;
    uint8_t    led1_enabled;
    uint8_t    slave1_mode;
    uint8_t    slave1_last_sample_pressed;
    uint8_t    slave1_stable_pressed;
    uint8_t    slave1_same_sample_count;
    uint32_t   heartbeat_count;
    CanModule  can_module;
    LedModule  slave1_led;
    char       mode_text[32];
    char       button_text[32];
    char       adc_text[48];
    char       can_input_text[48];
} AppCore;

InfraStatus AppCore_Init(AppCore *app);
void        AppCore_TaskHeartbeat(AppCore *app, uint32_t now_ms);
void        AppCore_TaskCan(AppCore *app, uint32_t now_ms);
void        AppCore_TaskButton(AppCore *app, uint32_t now_ms);
void        AppCore_TaskLed(AppCore *app, uint32_t now_ms);

#endif
