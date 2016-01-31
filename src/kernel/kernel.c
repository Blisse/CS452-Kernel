#include "kernel.h"

#include "cache.h"
#include "init.h"
#include <rtosc/assert.h>
#include "scheduler.h"
#include "syscall.h"
#include "task.h"
#include "trap.h"

static BOOLEAN g_exit;

static
inline
RT_STATUS
KernelCreateTask
    (
        IN TASK_PRIORITY priority, 
        IN TASK_START_FUNC startFunc
    )
{
    TASK_DESCRIPTOR* unused;

    return TaskCreate(SchedulerGetCurrentTask(), 
                      priority, 
                      startFunc,
                      &unused);
}

VOID
KernelInit
    (
        VOID
    )
{
    g_exit = FALSE;

    CacheInit();
    SchedulerInit();
    SyscallInit();
    TaskInit();
    TrapInstallHandler();

    VERIFY(RT_SUCCESS(KernelCreateTask(SystemPriority, InitTask)), 
           "Failed to create the init task \r\n");
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
    while(!g_exit)
    {
        TASK_DESCRIPTOR* nextTd;
        RT_STATUS status = SchedulerGetNextTask(&nextTd);

        if(RT_SUCCESS(status))
        {
            ASSERT(TaskValidate(nextTd), "Invalid task!  This likely indicates a stack overflow \r\n");

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
