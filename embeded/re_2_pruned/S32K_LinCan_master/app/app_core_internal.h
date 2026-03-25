/*
 * master AppCore 구현 내부에서만 쓰는 helper 선언이다.
 * coordinator 정책이 UI 문자열과 공통 CAN helper를 사용하더라도,
 * 외부에는 필요한 인터페이스만 노출되도록 범위를 줄인다.
 */
#ifndef APP_CORE_INTERNAL_H
#define APP_CORE_INTERNAL_H

#include "app/app_core.h"

void        AppCore_SetModeText(AppCore *app, const char *text);
void        AppCore_SetButtonText(AppCore *app, const char *text);
void        AppCore_SetAdcText(AppCore *app, const char *text);
void        AppCore_SetCanInputText(AppCore *app, const char *text);
void        AppCore_SetLinInputText(AppCore *app, const char *text);
void        AppCore_SetLinLinkText(AppCore *app, const char *text);
void        AppCore_SetResultText(AppCore *app, const char *text);
const char *AppCore_GetLinZoneText(uint8_t zone);
uint8_t     AppCore_QueueCanCommandCode(AppCore *app,
                                        uint8_t target_node_id,
                                        uint8_t command_code,
                                        uint8_t need_response);
InfraStatus AppCore_InitConsoleCan(AppCore *app);

#endif
