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

    TaskCreate(PRIORITY_SYSTEM, InitTask);
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
        TASK_DESCRIPTOR* nextTask = SchedulerGetNextTask();

        if(NULL != nextTask)
        {
            PVOID returnValue;

            if(!TaskValidate(nextTask))
            {
                ASSERT(FALSE, "Invalid task!  This likely indicats a buffer overflow \r\n");
            }

            returnValue = TrapReturn(nextTask->returnValue, nextTask->stack);

            TaskUpdate(nextTask, returnValue);
        }
        else
        {
            // No more tasks to run, quit the system
            g_exit = TRUE;
        }
    }
}
