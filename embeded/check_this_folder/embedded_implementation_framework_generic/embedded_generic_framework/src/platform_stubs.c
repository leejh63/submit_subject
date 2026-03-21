#include <stdarg.h>
#include "platform/platform_atomic.h"
#include "platform/platform_irq.h"
#include "platform/platform_log.h"

platform_irq_state_t platform_atomic_enter(void)
{
    return 0U;
}

void platform_atomic_leave(platform_irq_state_t state)
{
    (void)state;
}

void platform_irq_enable(void)
{
}

void platform_irq_disable(void)
{
}

void platform_log_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    platform_log_vprintf(fmt, ap);
    va_end(ap);
}

void platform_log_vprintf(const char *fmt, va_list ap)
{
    (void)fmt;
    (void)ap;
}
