#pragma once

#include <rt.h>

VOID
assert
    (
        BOOLEAN expr,
        INT line,
        STRING file
    );

#define T_ASSERT(expr) assert(expr, __LINE__, __FILE__)

#ifdef NLOCAL

    #ifdef NDEBUG

    #define ASSERT(expr)
    #define VERIFY(expr) (expr)

    #else

    #define ASSERT(expr) assert(expr, __LINE__, __FILE__)
    #define VERIFY(expr) ASSERT(expr)

    #endif

#else

    #define ASSERT(expr)
    #define VERIFY(expr) (expr)

#endif
