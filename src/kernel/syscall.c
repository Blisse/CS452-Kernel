#include "syscall.h"

#include "scheduler.h"

#define NUM_SYSCALLS 5

UINT g_systemCallTable[NUM_SYSCALLS];

VOID
SyscallInit
    (
        VOID
    )
{
    g_systemCallTable[0] = (UINT) SystemCreateTask;
    g_systemCallTable[1] = (UINT) SystemGetCurrentTaskId;
    g_systemCallTable[2] = (UINT) SystemGetCurrentParentTaskId;
    g_systemCallTable[3] = (UINT) SystemPassCurrentTask;
    g_systemCallTable[4] = (UINT) SystemDestroyCurrentTask;
}

INT
SystemCreateTask
    (
        IN INT priority,
        IN TASK_START_FUNC startFunc
    )
{
    INT status;
    TASK_DESCRIPTOR* td;

    status = TaskCreate(SystemGetCurrentTaskId(), priority, startFunc, &td);

    if (status > 0)
    {
        SchedulerAddTask(td);
    }

    return status;
}

INT
SystemGetCurrentTaskId
    (
        VOID
    )
{
    return SchedulerGetCurrentTask()->taskId;
}

INT
SystemGetCurrentParentTaskId
    (
        VOID
    )
{
    return SchedulerGetCurrentTask()->parentTaskId;
}

VOID
SystemPassCurrentTask
    (
        VOID
    )
{
    // This is intentionally left blank - it is a NOP
}

VOID
SystemDestroyCurrentTask
    (
        VOID
    )
{

}
