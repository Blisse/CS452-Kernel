#include "syscall.h"

#include <rtosc/assert.h>
#include "scheduler.h"

#define NUM_SYSCALLS 8

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
    g_systemCallTable[5] = (UINT) SystemSendMessage;
    g_systemCallTable[6] = (UINT) SystemReceiveMessage;
    g_systemCallTable[7] = (UINT) SystemReplyMessage;
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
    TASK_DESCRIPTOR* currentTask = SchedulerGetCurrentTask();
    INT parentTaskId = currentTask == NULL ? 0 : currentTask->taskId;

    status = TaskCreate(parentTaskId, 
                        priority, 
                        startFunc, 
                        &td);

    if (status > 0)
    {
        VERIFY(RT_SUCCESS(SchedulerAddTask(td)), 
               "New task failed to be added to scheduler \r\n");
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
    SchedulerGetCurrentTask()->state = ZombieState;
}
