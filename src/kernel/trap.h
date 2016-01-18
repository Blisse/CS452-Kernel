#pragma once

#include "rt.h"

extern
VOID
TrapInstallHandler
    (
        VOID
    );

extern
RT_STATUS
TrapReturn
    (
        IN RT_STATUS status, 
        IN UINT* stack
    );

extern
INT
GetR3
    (
        VOID
    );

extern
INT
GetSPSR
    (
        VOID
    );

extern
INT
GetSP
    (
        VOID
    );

extern
UINT*
GetUserSP
    (
        VOID
    );
