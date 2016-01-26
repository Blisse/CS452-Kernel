#pragma once

#include "rt.h"

#ifdef NDEBUG

#define ASSERT(expr, msg)
#define VERIFY(expr, msg) (expr)

#else

VOID
assert
    (
        BOOLEAN expr,
        STRING msg,
        INT line,
        STRING file
    );

#define ASSERT(expr, msg) assert(expr, msg, __LINE__, __FILE__)
#define VERIFY(expr, msg) ASSERT(expr, msg)

#endif
