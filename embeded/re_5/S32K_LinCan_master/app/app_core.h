/*
 * master 노드 전용 애플리케이션 context 헤더다.
 * coordinator가 실제로 사용하는 console, CAN, LIN 상태만 남겨,
 * 다른 노드용 필드 없이 핵심 흐름을 바로 읽을 수 있게 한다.
 */
#ifndef APP_CORE_H
#define APP_CORE_H

#include "app_console.h"
#include "../services/can_module.h"
#include "../core/infra_types.h"
#include "../services/lin_module.h"

typedef struct
{
    uint8_t    initialized;
    uint8_t    local_node_id;
    uint8_t    console_enabled;
    uint8_t    can_enabled;
    uint8_t    lin_enabled;
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
void        AppCore_TaskRender(AppCore *app, uint32_t now_ms);

#endif
