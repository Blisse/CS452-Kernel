#include "clock_server.h"

#include <bwio/bwio.h>
#include <rtosc/linked_list.h>
#include <rtosc/assert.h>

#include "name_server.h"
#include "scheduler.h"
#include "task_descriptor.h"

static volatile UINT g_ticks;

typedef enum _CLOCK_SERVER_REQUEST_TYPE
{
    TickRequest = 0,
    DelayRequest,
    TimeRequest,
    DelayUntilRequest,
} CLOCK_SERVER_REQUEST_TYPE;

typedef struct _CLOCK_SERVER_REQUEST
{
    CLOCK_SERVER_REQUEST_TYPE type;
    INT ticks;
} CLOCK_SERVER_REQUEST;

typedef struct _CLOCK_SERVER_DELAY_REQUEST
{
    INT taskId;
    UINT delayUntilTick;
} CLOCK_SERVER_DELAY_REQUEST;

static RT_LINKED_LIST g_delayedTasks;
static CLOCK_SERVER_DELAY_REQUEST g_delayRequests[NUM_TASK_DESCRIPTORS];

static
inline
VOID
ClockServerpResetDelayRequest
    (
        CLOCK_SERVER_DELAY_REQUEST* delayRequest
    )
{
    delayRequest->taskId = -1;
    delayRequest->delayUntilTick = 0;
}

static
VOID
ClockServerpInit
    (
        VOID
    )
{
    g_ticks = 0;

    RtLinkedListInit(&g_delayedTasks);

    UINT i;
    for (i = 0; i < NUM_TASK_DESCRIPTORS; i++)
    {
        ClockServerpResetDelayRequest(&g_delayRequests[i]);
    }
}

static
VOID
ClockNotifierpTask
    (
        VOID
    )
{
    INT clockServerTaskId = WhoIs(CLOCK_SERVER_NAME);
    CLOCK_SERVER_REQUEST notifyRequest = { TickRequest };

    while(1)
    {
        AwaitEvent(ClockEvent);
        Send(clockServerTaskId, &notifyRequest, sizeof(notifyRequest), NULL, 0);
    }
}

static
inline
CLOCK_SERVER_DELAY_REQUEST*
ClockServerpGetDelayRequest
    (
        RT_LINKED_LIST_NODE* delayRequestNode
    )
{
    return ((CLOCK_SERVER_DELAY_REQUEST*) delayRequestNode->data);
}

static
BOOLEAN
ClockServerpCanUnblockFrontDelayedTask
    (
        VOID
    )
{
    RT_LINKED_LIST_NODE* head;

    if (RT_SUCCESS(RtLinkedListPeekFront(&g_delayedTasks, &head)))
    {
        CLOCK_SERVER_DELAY_REQUEST* headDelayRequest = ClockServerpGetDelayRequest(head);

        ASSERT(headDelayRequest != NULL, "Should not have null delay request.");

        if (headDelayRequest->delayUntilTick <= g_ticks)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static
VOID
ClockServerpUnblockAllDelayedTasks
    (
        VOID
    )
{
    while (ClockServerpCanUnblockFrontDelayedTask())
    {
        RT_LINKED_LIST_NODE* head;
        VERIFY(RT_SUCCESS(RtLinkedListPeekAndPopFront(&g_delayedTasks, &head)), "Should be able to peek and pop front.");

        CLOCK_SERVER_DELAY_REQUEST* delayRequest = ClockServerpGetDelayRequest(head);
        Reply(delayRequest->taskId, NULL, 0);
    }
}

static
VOID
ClockServerpUpdateTick
    (
        INT notifierTaskId
    )
{
    g_ticks = g_ticks + 1;

    Reply(notifierTaskId, NULL, 0);

    ClockServerpUnblockAllDelayedTasks();
}

static
inline
BOOLEAN
ClockServerpIsDelayedLess
    (
        RT_LINKED_LIST_NODE* a,
        RT_LINKED_LIST_NODE* b
    )
{
    return ClockServerpGetDelayRequest(a)->delayUntilTick
        < ClockServerpGetDelayRequest(b)->delayUntilTick;
}

static
VOID
ClockServerpDelayRequestNode
    (
        RT_LINKED_LIST_NODE* delayRequestNode
    )
{
    RT_LINKED_LIST_NODE* current;

    RT_STATUS status = RtLinkedListPeekFront(&g_delayedTasks, &current);

    if (RT_SUCCESS(status))
    {
        while (current != NULL && ClockServerpIsDelayedLess(current, delayRequestNode))
        {
            current = current->next;
        }

    }

    if (current != NULL)
    {
        status = RtLinkedListInsertBetween(&g_delayedTasks,
                                   current->previous,
                                   current,
                                   delayRequestNode);
    }
    else
    {
        status = RtLinkedListPushBack(&g_delayedTasks, delayRequestNode);
    }

    ASSERT(RT_SUCCESS(status), "Could not delay request node.");
}

static
VOID
ClockServerpDelayTask
    (
        INT taskId,
        UINT delayUntilTick
    )
{
    TASK_DESCRIPTOR* td;

    VERIFY(RT_SUCCESS(TaskDescriptorGet(taskId, &td)), "Invalid task id.");

    ASSERT(td->state == ReplyBlockedState, "Task to delay must be reply-blocked.");

    CLOCK_SERVER_DELAY_REQUEST* delayRequest = &g_delayRequests[taskId % NUM_TASK_DESCRIPTORS];

    delayRequest->taskId = taskId;
    delayRequest->delayUntilTick = delayUntilTick;

    td->delayRequestNode.data = delayRequest;

    ClockServerpDelayRequestNode(&td->delayRequestNode);
}

static
VOID
ClockServerpReplyTicks
    (
        INT taskId
    )
{
    INT ticks = g_ticks;

    Reply(taskId, &ticks, sizeof(ticks));
}

static
VOID
ClockServerpTask
    (
        VOID
    )
{
    ClockServerpInit();

    RegisterAs(CLOCK_SERVER_NAME);

    INT clockNotifierTaskId = Create(HighestPriority, ClockNotifierpTask);
    ASSERT(clockNotifierTaskId > 0, "Could not create clock notifier task.");
    UNREFERENCED_PARAMETER(clockNotifierTaskId);

    while (1)
    {
        INT taskId;
        CLOCK_SERVER_REQUEST request;

        Receive(&taskId, &request, sizeof(request));

        switch (request.type)
        {
            case TickRequest:
                ClockServerpUpdateTick(taskId);
                break;
            case DelayRequest:
                ClockServerpDelayTask(taskId, g_ticks + request.ticks);
                break;
            case TimeRequest:
                ClockServerpReplyTicks(taskId);
                break;
            case DelayUntilRequest:
                ClockServerpDelayTask(taskId, request.ticks);
                break;
            default:
                ASSERT(FALSE, "Received invalid clock server request type.");
                break;
        }
    }
}

VOID
ClockServerCreateTask
    (
        VOID
    )
{
    INT clockServerTaskId = Create(Priority29, ClockServerpTask);
    ASSERT(clockServerTaskId > 0, "Could not create clock server task.");
    UNREFERENCED_PARAMETER(clockServerTaskId);
}

static
RT_STATUS
ClockServerSendRequest
    (
        CLOCK_SERVER_REQUEST* request
    )
{
    INT clockServerTaskId = WhoIs(CLOCK_SERVER_NAME);

    INT response = 0;

    if (RT_SUCCESS(Send(clockServerTaskId, request, sizeof(*request), &response, sizeof(response))))
    {
        return response;
    }

    return STATUS_FAILURE;
}

INT
Delay
    (
        INT ticks
    )
{
    CLOCK_SERVER_REQUEST request = { DelayRequest, ticks };
    return ClockServerSendRequest(&request);
}

INT
Time
    (
        VOID
    )
{
    CLOCK_SERVER_REQUEST request = { TimeRequest };
    return ClockServerSendRequest(&request);
}

INT
DelayUntil
    (
        INT ticks
    )
{
    CLOCK_SERVER_REQUEST request = { DelayUntilRequest, ticks };
    return ClockServerSendRequest(&request);
}
