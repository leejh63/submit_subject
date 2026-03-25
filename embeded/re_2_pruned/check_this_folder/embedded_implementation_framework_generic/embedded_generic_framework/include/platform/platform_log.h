#ifndef PLATFORM_LOG_H
#define PLATFORM_LOG_H

#include <stdarg.h>

void platform_log_printf(const char *fmt, ...);
void platform_log_vprintf(const char *fmt, va_list ap);

#endif
