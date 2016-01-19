#pragma once

#include "rt.h"
#include "task.h"

VOID
SchedulerInit
    (
        VOID
    );

VOID
SchedulerAddTask
    (
        IN TASK_DESCRIPTOR* task
    );

TASK_DESCRIPTOR*
SchedulerGetNextTask
    (
        VOID
    );

inline
TASK_DESCRIPTOR*
SchedulerGetCurrentTask
    (
        VOID
    );

VOID
SchedulerPassCurrentTask
    (
        VOID
    );
