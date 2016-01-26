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
}

static
inline
VOID
KernelpExit
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
    // Temporary for K1
    // Afterwards, set init task to SystemPriority
    SystemCreateTask(MediumPriority, InitTask);

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
            TaskUpdate(nextTd);

            // The task may have transitioned to a new state
            // due to interrupts, Exit(), etc.  Don't update
            // the state unless nothing happened to the task
            if(nextTd->state == RunningState)
            {
                nextTd->state = ReadyState;
            }
        }
        else if(STATUS_NOT_FOUND == status)
        {
            // No more tasks to run, quit the system
            KernelpExit();
        }
        else
        {
            ASSERT(FALSE, "Scheduling failed \r\n");
        }
    }
}
