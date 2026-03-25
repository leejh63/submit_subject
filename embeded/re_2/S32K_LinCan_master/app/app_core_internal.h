/*
 * AppCore 구현 내부에서만 사용하는 helper 선언 모음이다.
 * 이 선언들은 app 계층 내부에만 남겨두고,
 * 텍스트 갱신과 역할 조정, 공통 보조 함수들을 지원한다.
 */
#ifndef APP_CORE_INTERNAL_H
#define APP_CORE_INTERNAL_H

#include "app/app_core.h"

/*
 * slave1 내부 모드 식별값이다.
 * 이 값들은 app 계층 내부에서만 사용되며,
 * 콘솔에 표시되는 로컬 반응 노드 상태기계를 설명한다.
 */
#define APP_SLAVE1_MODE_NORMAL     0U
#define APP_SLAVE1_MODE_EMERGENCY  1U
#define APP_SLAVE1_MODE_ACK_BLINK  2U

/*
 * app 계층 내부에서만 쓰는 비공개 helper 선언이다.
 * 텍스트 갱신과 공통 유틸리티를,
 * runtime이 보는 공개 AppCore 헤더 밖으로 분리해 둔다.
 */
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
