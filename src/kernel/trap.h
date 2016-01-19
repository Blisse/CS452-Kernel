#pragma once

#include "rt.h"

extern
VOID
TrapInstallHandler
    (
        VOID
    );

extern
PVOID
TrapReturn
    (
        IN PVOID returnValue, 
        IN UINT* stack
    );
