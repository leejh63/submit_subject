/*
 * LIN sensor slave AppCore 내부 helper 선언이다.
 * 센서 노드는 console 없이도 상태 문자열을 내부에 유지하므로,
 * 필요한 텍스트 갱신 함수만 좁게 노출한다.
 */
#ifndef APP_CORE_INTERNAL_H
#define APP_CORE_INTERNAL_H

#include "app/app_core.h"

void AppCore_SetAdcText(AppCore *app, const char *text);
void AppCore_SetLinInputText(AppCore *app, const char *text);
void AppCore_SetLinLinkText(AppCore *app, const char *text);

#endif
