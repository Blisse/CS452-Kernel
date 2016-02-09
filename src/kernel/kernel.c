#include "kernel.h"

#include <init.h>
#include <rtosc/assert.h>

#include "cache.h"
#include "interrupt.h"
#include "performance.h"
#include "scheduler.h"
#include "syscall.h"
#include "trap.h"

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
    CacheInit();
    InterruptInit();
    PerformanceInit();
    SchedulerInit();
    SyscallInit();
    TaskInit();
    TrapInstallHandler();

    VERIFY(RT_SUCCESS(KernelCreateTask(SystemPriority, InitTask)));
}

VOID
KernelRun
    (
        VOID
    )
{
    KernelpInit();
    InterruptEnable();

    while(1)
    {
        TASK_DESCRIPTOR* nextTd;

        RT_STATUS status = SchedulerGetNextTask(&nextTd);

        if(RT_SUCCESS(status))
        {
            ASSERT(TaskValidate(nextTd));

            nextTd->state = RunningState;

            // Return to user mode
            PerformanceEnterTask();
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
            break;
        }
        else
        {
            ASSERT(FALSE);
        }
    }

    InterruptDisable();
}
