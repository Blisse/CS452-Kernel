#pragma once

#include "rt.h"
#include "task.h"

VOID
KernelRun
    (
        VOID
    );

extern
VOID
KernelLeave
    (
        IN UINT* stack
    );
