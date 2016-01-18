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
        IN UINT stack
    );
