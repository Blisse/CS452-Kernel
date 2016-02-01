#pragma once

#include "rt.h"
#include "task.h"

VOID
KernelInit
    (
        VOID
    );

VOID
KernelRun
    (
        VOID
    );

extern
VOID
KernelSaveUserContext
    (
        IN TASK_DESCRIPTOR* td, 
        IN UINT userPC
    );

extern
VOID
KernelLeave
    (
        IN UINT* stack
    );

extern
VOID
KernelEnter
    (
        VOID
    );
