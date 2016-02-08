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

extern TASK_DESCRIPTOR* g_currentTd;

static
inline
TASK_DESCRIPTOR*
SchedulerGetCurrentTask
    (
        VOID
    )
{
    return g_currentTd;
}
