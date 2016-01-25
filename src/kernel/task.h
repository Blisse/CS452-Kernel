#pragma once

#include "rt.h"
#include "task_descriptor.h"

VOID
TaskInit
    (
        VOID
    );

INT
TaskCreate
    (
        IN INT parentTaskId,
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc,
        OUT TASK_DESCRIPTOR** taskDescriptor
    );

inline
BOOLEAN
TaskValidate
    (
        IN TASK_DESCRIPTOR* task
    );

inline
VOID
TaskUpdate
    (
        IN TASK_DESCRIPTOR* task
    );
