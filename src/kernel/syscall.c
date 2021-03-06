#include "syscall.h"

#include <rtosc/assert.h>
#include "interrupt.h"
#include "ipc.h"
#include "performance.h"
#include "scheduler.h"
#include "task.h"

#define ERROR_SUCCESS 0
#define ERROR_PRIORITY_INVALID -1
#define ERROR_OUT_OF_SPACE -2
#define ERROR_INVALID_TASK -1
#define ERROR_DEAD_TASK -2
#define ERROR_TASK_NOT_REPLY_BLOCKED -3
#define ERROR_INVALID_EVENT -1

#define NUM_SYSCALLS 10

UINT g_systemCallTable[NUM_SYSCALLS];

static
INT
SystemCreateTask
    (
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc
    )
{
    TASK_DESCRIPTOR* td;
    RT_STATUS status = TaskCreate(SchedulerGetCurrentTask(),
                                  priority,
                                  startFunc,
                                  &td);

    switch(status)
    {
        case STATUS_SUCCESS:
            return td->taskId;

        case STATUS_INVALID_PARAMETER:
            return ERROR_PRIORITY_INVALID;

        case STATUS_BUFFER_TOO_SMALL:
            return ERROR_OUT_OF_SPACE;

        default:
            ASSERT(FALSE);
            return 0;
    }
}

static
INT
SystemGetCurrentTaskId()
{
    return SchedulerGetCurrentTask()->taskId;
}

static
INT
SystemGetCurrentParentTaskId()
{
    return SchedulerGetCurrentTask()->parentTaskId;
}

static
VOID
SystemPassCurrentTask()
{
    // This is intentionally left blank - it is a NOP
}

static
VOID
SystemDestroyCurrentTask()
{
    TaskDestroy(SchedulerGetCurrentTask());
}

static
INT
SystemSendMessage
    (
        IN INT taskId,
        IN PVOID message,
        IN INT messageLength,
        IN PVOID reply,
        IN INT replyLength
    )
{
    TASK_DESCRIPTOR* from = SchedulerGetCurrentTask();
    TASK_DESCRIPTOR* to;
    RT_STATUS status = TaskDescriptorGet(taskId, &to);

    if (RT_SUCCESS(status))
    {
        status = IpcSend(from, to, message, messageLength, reply, replyLength);
    }

    switch(status)
    {
        case STATUS_SUCCESS:
            return ERROR_SUCCESS;

        case STATUS_INVALID_PARAMETER:
            return ERROR_INVALID_TASK;

        case STATUS_NOT_FOUND:
            return ERROR_DEAD_TASK;

        default:
            ASSERT(FALSE);
            return 0;
    }
}

static
INT
SystemReceiveMessage
    (
        OUT INT* senderId,
        OUT PVOID buffer,
        IN INT bufferLength
    )
{
    TASK_DESCRIPTOR* currentTask = SchedulerGetCurrentTask();
    INT bytesReceived;

    VERIFY(RT_SUCCESS(IpcReceive(currentTask,
                                 senderId,
                                 buffer,
                                 bufferLength,
                                 &bytesReceived)));

    return bytesReceived;
}

static
INT
SystemReplyMessage
    (
        IN INT taskId,
        IN PVOID reply,
        IN INT replyLength
    )
{
    TASK_DESCRIPTOR* from = SchedulerGetCurrentTask();
    TASK_DESCRIPTOR* to;
    RT_STATUS status = TaskDescriptorGet(taskId, &to);

    if (RT_SUCCESS(status))
    {
        status = IpcReply(from, to, reply, replyLength);
    }

    switch(status)
    {
        case STATUS_SUCCESS:
            return ERROR_SUCCESS;

        case STATUS_INVALID_PARAMETER:
            return ERROR_INVALID_TASK;

        case STATUS_NOT_FOUND:
            return ERROR_DEAD_TASK;

        case STATUS_INVALID_STATE:
            return ERROR_TASK_NOT_REPLY_BLOCKED;

        default:
            ASSERT(FALSE);
            return 0;
    }
}

static
INT
SystemAwaitEvent
    (
        IN EVENT event
    )
{
    RT_STATUS status = InterruptAwaitEvent(SchedulerGetCurrentTask(), event);

    switch(status)
    {
        case STATUS_SUCCESS:
            return ERROR_SUCCESS;

        case STATUS_INVALID_PARAMETER:
            return ERROR_INVALID_EVENT;

        default:
            ASSERT(FALSE);
            return 0;
    }
}

static
INT
SystemQueryPerformance
    (
        IN INT taskId,
        OUT TASK_PERFORMANCE* performance
    )
{
    RT_STATUS status = PerformanceGet(taskId, performance);

    switch (status)
    {
        case STATUS_SUCCESS:
            return ERROR_SUCCESS;

        case STATUS_FAILURE:
            return ERROR_INVALID_TASK;

        default:
            ASSERT(FALSE);
            return 0;
    }
}

VOID
SyscallInit()
{
    g_systemCallTable[0] = (UINT) SystemCreateTask;
    g_systemCallTable[1] = (UINT) SystemGetCurrentTaskId;
    g_systemCallTable[2] = (UINT) SystemGetCurrentParentTaskId;
    g_systemCallTable[3] = (UINT) SystemPassCurrentTask;
    g_systemCallTable[4] = (UINT) SystemDestroyCurrentTask;
    g_systemCallTable[5] = (UINT) SystemSendMessage;
    g_systemCallTable[6] = (UINT) SystemReceiveMessage;
    g_systemCallTable[7] = (UINT) SystemReplyMessage;
    g_systemCallTable[8] = (UINT) SystemAwaitEvent;
    g_systemCallTable[9] = (UINT) SystemQueryPerformance;
}
