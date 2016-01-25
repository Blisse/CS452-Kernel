#pragma once

#include "rt.h"
#include "task.h"

VOID
KernelInit
    (
        VOID
    );

inline
VOID
KernelExit
    (
        VOID
    );

VOID
KernelRun
    (
        VOID
    );
