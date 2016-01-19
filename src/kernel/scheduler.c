#include "scheduler.h"

static TASK_DESCRIPTOR* g_CurrentTask;

VOID
SchedulerInit
    (
        VOID
    )
{
    g_CurrentTask = NULL;
}

VOID
SchedulerAddTask
    (
        IN TASK_DESCRIPTOR* task
    )
{

}

TASK_DESCRIPTOR*
SchedulerGetNextTask
    (
        VOID
    )
{
    // TODO: Run the scheduler
    return g_CurrentTask;
}

inline
TASK_DESCRIPTOR*
SchedulerGetCurrentTask
    (
        VOID
    )
{
    return g_CurrentTask;
}

VOID
SchedulerPassCurrentTask
    (
        VOID
    )
{
    // Probably a nop?
}
