#pragma once

#include "rt.h"

typedef struct _STACK
{
    UINT id;
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

inline
BOOLEAN
StackVerify
    (
        IN STACK* stack
    );
