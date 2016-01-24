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

    TaskCreate(SystemPriority, InitTask);
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
        TASK_DESCRIPTOR* nextTask;
        RT_STATUS status = SchedulerGetNextTask(&nextTask);

        if(RT_SUCCESS(status))
        {
            if(!TaskValidate(nextTask))
            {
                ASSERT(FALSE, "Invalid task!  This likely indicates a buffer overflow \r\n");
            }

            nextTask->state = Running;

            // Return to user mode
            TrapReturn(nextTask->stack);

            // This will execute once we return back to kernel mode
            // Update the task that just ran
            nextTask->state = Ready;
            TaskUpdate(nextTask);
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
