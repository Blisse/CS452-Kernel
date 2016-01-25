#pragma once

#include "rt.h"
#include "task.h"

VOID
SyscallInit
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
SystemPassCurrentTask
    (
        VOID
    );

VOID
SystemDestroyCurrentTask
    (
        VOID
    );
