#include "syscall.h"

#include <bwio/bwio.h>
#include <rtosc/assert.h>
#include <rtosc/string.h>
#include "scheduler.h"
#include "task_descriptor.h"

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
    TaskDescriptorDestroy(SchedulerGetCurrentTask()->taskId);
}

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
    bwprintf(BWCOM2, "Send with : %d - %d - %d - %d - %d", taskId, message, messageLength, reply, replyLength);

    TASK_DESCRIPTOR* senderTd = SchedulerGetCurrentTask();

    TASK_DESCRIPTOR* receiverTd;
    if (RT_FAILURE(TaskDescriptorGet(taskId, &receiverTd)))
    {
        return -1;
    }

    if (RT_FAILURE(TaskDescriptorMessagePush(receiverTd, senderTd->taskId, message, messageLength)))
    {
        return -1;
    }

    senderTd->replyMessageBuffer = reply;
    senderTd->replyMessageLength = replyLength;
    senderTd->state = ReceiveBlockedState;

    if (receiverTd->state == SendBlockedState)
    {
        senderTd->state = ReplyBlockedState;

        receiverTd->state = ReadyState;
        *(receiverTd->receiveMessageSenderId) = senderTd->taskId;
        RtMemcpy(receiverTd->receiveMessageBuffer, message, receiverTd->receiveMessageLength);
        TaskDescriptorSetReturnValue(receiverTd, messageLength);

        VERIFY(RT_SUCCESS(SchedulerAddTask(receiverTd)), "Failed to add task to scheduler\r\n");
    }

    return -1;
}

INT
SystemReceiveMessage
    (
        OUT INT* taskId,
        OUT PVOID message,
        IN INT messageLength
    )
{
    bwprintf(BWCOM2, "Receive with : %d - %d - %d", taskId, message, messageLength);

    TASK_DESCRIPTOR* receiverTd = SchedulerGetCurrentTask();

    TASK_MESSAGE receivedMessage;
    if (RT_FAILURE(TaskDescriptorMessagePop(receiverTd, &receivedMessage)))
    {
        return -1;
    }

    TASK_DESCRIPTOR* senderTd;
    if (RT_FAILURE(TaskDescriptorGet(receivedMessage.senderId, &senderTd)))
    {
        receiverTd->state = SendBlockedState;
        return -1;
    }

    senderTd->state = ReplyBlockedState;

    *taskId = receivedMessage.senderId;
    RtMemcpy(message, receivedMessage.message, messageLength);
    return receivedMessage.messageLength;
}

INT
SystemReplyMessage
    (
        IN INT taskId,
        IN PVOID reply,
        IN INT replyLength
    )
{
    bwprintf(BWCOM2, "Reply with : %d - %d - %d", taskId, reply, replyLength);

    TASK_DESCRIPTOR* senderTd;
    if (RT_FAILURE(TaskDescriptorGet(taskId, &senderTd)))
    {
        return -1;
    }

    if (senderTd->state != ReplyBlockedState)
    {
        return -1;
    }

    if (senderTd->replyMessageLength < replyLength)
    {
        return -1;
    }

    RtMemcpy(senderTd->replyMessageBuffer, reply, replyLength);
    senderTd->state = ReadyState;
    TaskDescriptorSetReturnValue(senderTd, replyLength);
    VERIFY(RT_SUCCESS(SchedulerAddTask(senderTd)), "Failed to add task to scheduler\r\n");

    return 0;
}
