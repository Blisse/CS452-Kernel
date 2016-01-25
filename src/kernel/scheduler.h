#pragma once

#include "rt.h"
#include "task.h"

VOID
SchedulerInit
    (
        VOID
    );

inline
RT_STATUS
SchedulerAddTask
    (
        IN TASK_DESCRIPTOR* td
    );

RT_STATUS
SchedulerGetNextTask
    (
        OUT TASK_DESCRIPTOR** td
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
