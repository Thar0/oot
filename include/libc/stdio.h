#ifndef STDIO_H
#define STDIO_H

#include "stdarg.h"

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

__attribute__((format(printf, 2, 3)))
int sprintf(char* dst, const char* fmt, ...);

__attribute__((format(printf, 2, 0)))
int vsprintf(char* dst, const char* fmt, va_list args);

#endif
