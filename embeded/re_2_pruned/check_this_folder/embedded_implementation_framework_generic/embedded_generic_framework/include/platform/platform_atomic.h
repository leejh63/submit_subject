#ifndef PLATFORM_ATOMIC_H
#define PLATFORM_ATOMIC_H

#include <stdint.h>

/*
 * 실제 프로젝트에서는 여기에
 * - IRQ disable/restore
 * - C11 atomic wrapper
 * - compiler barrier
 * - memory barrier
 * 를 플랫폼에 맞게 채운다.
 */

typedef uint32_t platform_irq_state_t;

platform_irq_state_t platform_atomic_enter(void);
void platform_atomic_leave(platform_irq_state_t state);

#endif
