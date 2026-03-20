#ifndef RUNTIME_H
#define RUNTIME_H

#include "status.h"

/*
 * 전체 시스템 부팅 및 주기 실행의 최상위 진입점.
 * main()은 이 헤더의 함수만 직접 사용한다.
 */

/* 모든 전역 컨텍스트와 주변 장치를 초기화한다. */
status_t Runtime_Init(void);

/* tick 기준으로 태스크 테이블을 실행한다. */
void Runtime_Run(void);

/* 치명적 실패 시 빠져나오지 않는 무한 루프. */
void Runtime_FaultLoop(void);

#endif
