#include "ipc.h"

#include <rtosc/string.h>
#include "scheduler.h"

#define ERROR_TRANSACTION_NOT_FINISHED -3

#define MAILBOX_SIZE NUM_TASKS

typedef struct _PENDING_MESSAGE 
{
    TASK_DESCRIPTOR* from;
    PVOID message;
    INT messageLength;
} PENDING_MESSAGE;

typedef struct _PENDING_RECEIVE
{
    INT* senderId;
    PVOID buffer;
    INT bufferLength;
} PENDING_RECEIVE;

static PENDING_MESSAGE g_mailboxes[NUM_TASKS][MAILBOX_SIZE];

VOID
IpcInitializeMailbox
    (
        IN TASK_DESCRIPTOR* td
    )
{
    RtCircularBufferInit(&td->mailbox,
                         g_mailboxes[td->taskId % NUM_TASKS], 
                         MAILBOX_SIZE);
}

VOID
IpcDrainMailbox
    (
        IN TASK_DESCRIPTOR* td
    )
{
    while(!RtCircularBufferIsEmpty(&td->mailbox))
    {
        PENDING_MESSAGE pendingMessage;
        RT_STATUS status = RtCircularBufferPeekAndPop(&td->mailbox,
                                                        &pendingMessage,
                                                        sizeof(pendingMessage));

        if(RT_SUCCESS(status))
        {
            TASK_DESCRIPTOR* from = pendingMessage.from;

            // The Send-Receive-Reply transaction could not be completed
            TaskSetReturnValue(from, ERROR_TRANSACTION_NOT_FINISHED);
            from->state = ReadyState;
            SchedulerAddTask(from);
        }
    }
}

RT_STATUS
IpcSend
    (
        IN TASK_DESCRIPTOR* from, 
        IN TASK_DESCRIPTOR* to, 
        IN PVOID message, 
        IN INT messageLength, 
        IN PVOID replyBuffer, 
        IN INT replyBufferLength
    )
{
    RT_STATUS status;

    if(to->state == SendBlockedState)
    {
        PENDING_RECEIVE pendingReceive;
        INT length;

        TaskRetrieveAsyncParameter(to, &pendingReceive, sizeof(pendingReceive));
        
        // Figure out how many bytes we actually want to copy
        // TODO: We should probably do something if there are excess bytes
        length = min(messageLength, pendingReceive.bufferLength);

        // Perform the copy
        RtMemcpy(pendingReceive.buffer, message, length);

        // Finish the Receive() system call
        *(pendingReceive.senderId) = from->taskId;
        TaskSetReturnValue(to, length);

        // Update states and reschedule the target task
        from->state = ReplyBlockedState;
        to->state = ReadyState;
        status = SchedulerAddTask(to);
    }
    else
    {
        PENDING_MESSAGE pendingMessage = { from, message, messageLength };

        // Store the message so it can be picked up later
        from->state = ReceiveBlockedState;
        status = RtCircularBufferPush(&to->mailbox,
                                      &pendingMessage,
                                      sizeof(pendingMessage));
    }

    if(RT_SUCCESS(status))
    {
        PENDING_RECEIVE pendingReceive = { NULL, replyBuffer, replyBufferLength };

        TaskStoreAsyncParameter(from, &pendingReceive, sizeof(pendingReceive));
    }

    return status;
}

RT_STATUS
IpcReceive
    (
        IN TASK_DESCRIPTOR* td, 
        IN INT* sendingTaskId,
        IN PVOID buffer, 
        IN INT bufferLength, 
        OUT INT* bytesReceived
    )
{
    RT_STATUS status;

    if(!RtCircularBufferIsEmpty(&td->mailbox))
    {
        PENDING_MESSAGE pendingMessage;

        status = RtCircularBufferPeekAndPop(&td->mailbox,
                                            &pendingMessage,
                                            sizeof(pendingMessage));

        if(RT_SUCCESS(status))
        {
            // Figure out how many bytes we actually want to copy
            // TODO: We should probably do something if there are excess bytes
            INT length = min(bufferLength, pendingMessage.messageLength);

            // Perform the copy
            RtMemcpy(buffer, 
                     pendingMessage.message,
                     length);

            // Finish the system call
            *sendingTaskId = pendingMessage.from->taskId;
            *bytesReceived = length;

            // Update states
            pendingMessage.from->state = ReplyBlockedState;
        }
    }
    else
    {
        PENDING_RECEIVE pendingReceive = { sendingTaskId, buffer, bufferLength };

        TaskStoreAsyncParameter(td, &pendingReceive, sizeof(pendingReceive));

        status = STATUS_SUCCESS;
        td->state = SendBlockedState;
        *bytesReceived = 0;
    }

    return status;
}

RT_STATUS
IpcReply
    (
        IN TASK_DESCRIPTOR* from, 
        IN TASK_DESCRIPTOR* to, 
        IN PVOID message, 
        IN INT messageLength
    )
{
    RT_STATUS status;

    if(to->state == ReplyBlockedState)
    {
        PENDING_RECEIVE pendingReceive;
        INT length;

        TaskRetrieveAsyncParameter(to, &pendingReceive, sizeof(pendingReceive));

        // Figure out how many bytes we actually want to copy
        // TODO: We should probably do something if there are excess bytes
        length = min(messageLength, pendingReceive.bufferLength);

        // Perform the copy
        RtMemcpy(pendingReceive.buffer, message, length);

        // Finish the Send() system call
        TaskSetReturnValue(to, length);

        // Update states and reschedule the target task
        to->state = ReadyState;
        status = SchedulerAddTask(to);
    }
    else
    {
        status = STATUS_INVALID_STATE;
    }

    return status;
}
