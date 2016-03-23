#pragma once

#include <rt.h>

typedef struct _STACK
{
    UINT* top;
    UINT size;
} STACK;

RT_STATUS
StackInit();

RT_STATUS
StackAllocate
    (
        OUT STACK** stack
    );

RT_STATUS
StackDeallocate
    (
        IN STACK* stack
    );
