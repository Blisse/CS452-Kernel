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

INT
SystemCreateTask
    (
        IN INT priority,
        IN TASK_START_FUNC startFunc
    );

INT
SystemGetCurrentTaskId
    (
        VOID
    );

INT
SystemGetCurrentParentTaskId
    (
        VOID
    );

VOID
SystemDestroyCurrentTask
    (
        VOID
    );
