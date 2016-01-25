#include "kernel.h"

#include <rtosc/assert.h>
#include "scheduler.h"
#include "syscall.h"
#include "task.h"
#include "trap.h"
#include "init.h"

static BOOLEAN g_exit;

VOID
KernelInit
    (
        VOID
    )
{
    g_exit = FALSE;

    SchedulerInit();
    SyscallInit();
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

            nextTd->state = RunningState;

            // Return to user mode
            TrapReturn(nextTd->stackPointer);

            // This will execute once we return back to kernel mode
            // Update the task that just ran
            nextTd->state = ReadyState;
            TaskUpdate(nextTd);
        }
        else if(STATUS_NOT_FOUND == status)
        {
            // No more tasks to run, quit the system
            KernelExit();
        }
        else
        {
            ASSERT(FALSE, "Scheduling failed \r\n");
        }
    }
}
