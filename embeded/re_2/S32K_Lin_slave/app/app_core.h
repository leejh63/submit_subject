/*
 * 공통 애플리케이션 오케스트레이션 상태를 담는 헤더다.
 * AppCore는 역할 플래그, 모듈 인스턴스, 카운터,
 * UI 문자열을 한곳에 모아 runtime이 하나의 context만 다루게 한다.
 */
#ifndef APP_CORE_H
#define APP_CORE_H

#include "adc/adc_module.h"
#include "app/app_console.h"
#include "can/can_module.h"
#include "infra/infra_types.h"
#include "led/led_module.h"
#include "lin/lin_module.h"

/*
 * 모든 펌웨어 역할이 공유하는 애플리케이션 context다.
 * enable 플래그와 역할 상태, 모듈 인스턴스, 카운터,
 * UART 콘솔 렌더러가 보여줄 문자열을 함께 가진다.
 */
typedef struct
{
    uint8_t    initialized;
    uint8_t    role;
    uint8_t    local_node_id;
    uint8_t    console_enabled;
    uint8_t    can_enabled;
    uint8_t    lin_enabled;
    uint8_t    adc_enabled;
    uint8_t    led1_enabled;
    uint8_t    led2_enabled;
    uint8_t    slave1_mode;
    uint8_t    slave1_last_sample_pressed;
    uint8_t    slave1_stable_pressed;
    uint8_t    slave1_same_sample_count;
    uint8_t    master_emergency_active;
    uint8_t    master_slave1_ok_pending;
    uint8_t    can_last_activity;
    uint8_t    lin_last_reported_zone;
    uint8_t    lin_last_reported_lock;
    uint32_t   heartbeat_count;
    uint32_t   uart_task_count;
    uint32_t   can_task_count;
    AppConsole console;
    CanModule  can_module;
    LinModule  lin_module;
    LedModule  slave1_led;
    LedModule  slave2_led;
    AdcModule  adc_module;
    char       mode_text[32];
    char       button_text[32];
    char       adc_text[48];
    char       can_input_text[48];
    char       lin_input_text[48];
    char       lin_link_text[32];
} AppCore;

InfraStatus AppCore_Init(AppCore *app);
void        AppCore_OnTickIsr(void *context);
void        AppCore_TaskHeartbeat(AppCore *app, uint32_t now_ms);
void        AppCore_TaskUart(AppCore *app, uint32_t now_ms);
void        AppCore_TaskCan(AppCore *app, uint32_t now_ms);
void        AppCore_TaskLinFast(AppCore *app, uint32_t now_ms);
void        AppCore_TaskLinPoll(AppCore *app, uint32_t now_ms);
void        AppCore_TaskButton(AppCore *app, uint32_t now_ms);
void        AppCore_TaskLed(AppCore *app, uint32_t now_ms);
void        AppCore_TaskAdc(AppCore *app, uint32_t now_ms);
void        AppCore_TaskRender(AppCore *app, uint32_t now_ms);

#endif
