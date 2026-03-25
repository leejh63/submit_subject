/*
 * 펌웨어 이미지의 최상위 runtime 인터페이스다.
 * 애플리케이션은 이 함수들만 호출하면 시스템을 초기화하고,
 * 무한 cooperative scheduler로 진입할 수 있다.
 */
#ifndef RUNTIME_H
#define RUNTIME_H

#include "app/app_core.h"
#include "infra/infra_types.h"

/*
 * runtime 제어를 위한 공개 인터페이스다.
 * 호출자는 이 함수들로 공통 AppCore를 만들고,
 * scheduler에 진입하며, 현재 runtime context를 확인할 수 있다.
 */
InfraStatus    Runtime_Init(void);
void           Runtime_Run(void);
const AppCore *Runtime_GetApp(void);

#endif
