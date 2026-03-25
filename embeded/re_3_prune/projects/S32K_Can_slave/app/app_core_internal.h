/*
 * CAN 현장 반응 slave 내부 helper 선언이다.
 * slave1 정책이 모드 문자열과 CAN helper를 사용하더라도,
 * 외부에는 실제 task 진입점만 보이도록 보조 함수를 분리한다.
 */
#ifndef APP_CORE_INTERNAL_H
#define APP_CORE_INTERNAL_H

#include "app_core.h"

#define APP_SLAVE1_MODE_NORMAL     0U
#define APP_SLAVE1_MODE_EMERGENCY  1U
#define APP_SLAVE1_MODE_ACK_BLINK  2U

void        AppCore_SetModeText(AppCore *app, const char *text);
void        AppCore_SetButtonText(AppCore *app, const char *text);
void        AppCore_SetAdcText(AppCore *app, const char *text);
void        AppCore_SetCanInputText(AppCore *app, const char *text);
void        AppCore_SetResultText(AppCore *app, const char *text);
uint8_t     AppCore_QueueCanCommandCode(AppCore *app,
                                        uint8_t target_node_id,
                                        uint8_t command_code,
                                        uint8_t need_response);
InfraStatus AppCore_InitConsoleCan(AppCore *app);

#endif
