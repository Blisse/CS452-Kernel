#include "syscall.h"

#include "scheduler.h"

#define NUM_SYSCALLS 5

UINT* g_systemCallTable[NUM_SYSCALLS];

VOID
SyscallInit
    (
        VOID
    )
{
    g_systemCallTable[0] = (UINT*) SystemCreateTask;
    g_systemCallTable[1] = (UINT*) SystemGetCurrentTaskId;
    g_systemCallTable[2] = (UINT*) SystemGetCurrentParentTaskId;
    g_systemCallTable[3] = (UINT*) SystemPassCurrentTask;
    g_systemCallTable[4] = (UINT*) SystemDestroyCurrentTask;
}

INT
SystemCreateTask
    (
        IN INT priority,
        IN TASK_START_FUNC startFunc
    )
{
    TASK_DESCRIPTOR* td;

    TaskCreate(SystemGetCurrentTaskId(), priority, startFunc, &td);

    return 0;
}

INT
SystemGetCurrentTaskId
    (
        VOID
    )
{
    return 4;
}

INT
SystemGetCurrentParentTaskId
    (
        VOID
    )
{
    return 2;
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
