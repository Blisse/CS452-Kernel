#pragma once

#include "rt.h"

extern
VOID
TrapInstallHandler
    (
        VOID
    );

extern
VOID
TrapReturn
    (
        IN UINT* stack
    );
