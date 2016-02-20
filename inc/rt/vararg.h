#pragma once

#include "types.h"

typedef CHAR* VA_LIST;

#define __VA_ARGSIZ(t)  \
        (((sizeof(t) + sizeof(INT) - 1) / sizeof(INT)) * sizeof(INT))

#define VA_START(ap, pN) ((ap) = ((VA_LIST) __builtin_next_arg(pN)))

#define VA_END(ap)  ((VOID)0)

#define VA_ARG(ap, t)   \
         (((ap) = (ap) + __VA_ARGSIZ(t)), *((t*) (PVOID) ((ap) - __VA_ARGSIZ(t))))
