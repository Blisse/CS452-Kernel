#include "kernel.h"

#include <init.h>
#include <rtosc/assert.h>

#include "cache.h"
#include "idle.h"
#include "interrupt.h"
#include "performance.h"
#include "scheduler.h"
#include "syscall.h"
#include "trap.h"

static BOOLEAN g_running;

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

    return TaskCreate(NULL,
                      priority,
                      startFunc,
                      &unused);
}

static
VOID
KernelpInit
    (
        VOID
    )
{
    g_running = TRUE;

    CacheInit();
    IdleInit();
    InterruptInit();
    PerformanceInit();
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
    g_running = FALSE;
}

static
inline
BOOLEAN
KernelpIsRunning
    (
        VOID
    )
{
    return g_running;
}

VOID
KernelRun
    (
        VOID
    )
{
    KernelpInit();

    InterruptEnable();

    while(KernelpIsRunning())
    {
        TASK_DESCRIPTOR* nextTd;

        RT_STATUS status = SchedulerGetNextTask(&nextTd);

        if(RT_SUCCESS(status))
        {
            ASSERT(TaskValidate(nextTd), "Invalid task!  This likely indicates a stack overflow \r\n");

            nextTd->state = RunningState;

            PerformanceEnterTask();
            // Return to user mode
            KernelLeave(nextTd->stackPointer);
            PerformanceExitTask(nextTd->taskId);

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

    InterruptDisable();
}
