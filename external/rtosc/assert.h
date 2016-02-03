#pragma once

#include "rt.h"

VOID
assert
    (
        BOOLEAN expr,
        STRING msg,
        INT line,
        STRING file
    );

#define T_ASSERT(expr, msg) assert(expr, msg, __LINE__, __FILE__)

#ifdef NDEBUG

#define ASSERT(expr, msg)
#define VERIFY(expr, msg) (expr)

#else

#define ASSERT(expr, msg) assert(expr, msg, __LINE__, __FILE__)
#define VERIFY(expr, msg) ASSERT(expr, msg)

#endif
