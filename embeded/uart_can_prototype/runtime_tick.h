#ifndef RUNTIME_TICK_H
#define RUNTIME_TICK_H

#include <stdint.h>

#include "status.h"

/*
 * 시스템의 millisecond soft tick 인터페이스.
 * LPTMR interrupt를 이용해 g_runtimeTickMs를 증가시키는 얇은 래퍼다.
 */

/* tick 타이머와 IRQ를 초기화한다. */
status_t RuntimeTick_Init(void);

/* 현재 누적 tick(ms)을 반환한다. */
uint32_t RuntimeTick_GetMs(void);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (runtime_tick.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [runtime_tick.c]
 * - static void RuntimeTick_IrqHandler(void);
 *   : LPTMR interrupt에서 1ms tick 카운터를 증가시킨다.
 */
#endif
