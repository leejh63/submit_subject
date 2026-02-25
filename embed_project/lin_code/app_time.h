// app_time.h

#ifndef APP_TIME_H_
#define APP_TIME_H_

#pragma once

#include <stdint.h>

#define TIMER_COMPARE_VAL (uint16_t)(2000U)
#define TIMER_TICKS_1US   (uint16_t)(4U)

uint32_t app_time_now_tick500us(void);

// ISR/SDK callback prototypes (defined in app_time.c)
void LPTMR_ISR(void);

uint32_t lin1TimerGetTimeIntervalCallback0(uint32_t *ns);


#endif /* APP_TIME_H_ */
