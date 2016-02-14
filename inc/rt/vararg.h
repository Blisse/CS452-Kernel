#pragma once

#include "types.h"

typedef CHAR *va_list;

#define __va_argsiz(t)  \
        (((sizeof(t) + sizeof(INT) - 1) / sizeof(INT)) * sizeof(INT))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)  ((VOID)0)

#define va_arg(ap, t)   \
         (((ap) = (ap) + __va_argsiz(t)), *((t*) (PVOID) ((ap) - __va_argsiz(t))))
