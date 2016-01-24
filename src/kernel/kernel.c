#include "kernel.h"

#include <rtosc/assert.h>
#include "scheduler.h"
#include "task.h"
#include "trap.h"

extern
VOID
InitTask
    (
        VOID
    );

static BOOLEAN g_exit;

VOID
KernelInit
    (
        VOID
    )
{
    g_exit = FALSE;

    SchedulerInit();
    TaskInit();
    TrapInstallHandler();

    SystemCreateTask(SystemPriority, InitTask);
}

inline
VOID
KernelExit
    (
        VOID
    )
{
    g_exit = TRUE;
}

VOID
KernelRun
    (
        VOID
    )
{
    while(!g_exit)
    {
        TASK_DESCRIPTOR* nextTd;
        RT_STATUS status = SchedulerGetNextTask(&nextTd);

        if(RT_SUCCESS(status))
        {
            ASSERT(TaskValidate(nextTd), "Invalid task!  This likely indicates a buffer overflow \r\n");

            nextTd->state = Running;

            // Return to user mode
            TrapReturn(nextTd->stack);

            // This will execute once we return back to kernel mode
            // Update the task that just ran
            nextTd->state = Ready;
            TaskUpdate(nextTd);
        }
        else if(STATUS_NOT_FOUND == status)
        {
            // No more tasks to run, quit the system
            g_exit = TRUE;
        }
        else
        {
            ASSERT(FALSE, "Scheduling failed \r\n");
        }
    }
}

INT
SystemCreateTask
    (
        IN INT priority,
        IN TASK_START_FUNC startFunc
    )
{
    return 0;
}

INT
SystemGetCurrentTaskId
    (
        VOID
    )
{
    return 0;
}

INT
SystemGetCurrentParentTaskId
    (
        VOID
    )
{
    return 0;
}

VOID
SystemDestroyCurrentTask
    (
        VOID
    )
{
}
