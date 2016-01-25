#pragma once

#include "rt.h"

typedef struct _STACK
{
    INT id;
    UINT* top;
    UINT size;
} STACK;

RT_STATUS
StackInit
    (
        VOID
    );

RT_STATUS
StackGet
    (
        STACK* stack
    );

inline
RT_STATUS
StackReturn
    (
        STACK* stack
    );
