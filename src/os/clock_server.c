#include "clock_server.h"

#include <bwio/bwio.h>
#include <rtosc/linked_list.h>
#include <rtosc/assert.h>

#include "name_server.h"
#include "scheduler.h"
#include "task_descriptor.h"

#define CLOCK_SERVER_NAME "clk"

static volatile UINT g_ticks;

typedef enum _CLOCK_SERVER_REQUEST_TYPE
{
    NotifyTickRequest = 0,
    DelayRequest,
    TickRequest,
    DelayUntilRequest,
} CLOCK_SERVER_REQUEST_TYPE;

typedef struct _CLOCK_SERVER_REQUEST
{
    CLOCK_SERVER_REQUEST_TYPE type;
    TASK_DESCRIPTOR* td;
    INT ticks;
} CLOCK_SERVER_REQUEST;

typedef struct _CLOCK_SERVER_DELAY_REQUEST
{
    TASK_DESCRIPTOR* td;
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
    delayRequest->td = NULL;
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

    while(1)
    {
        AwaitEvent(ClockEvent);

        Send(clockServerTaskId, NULL, 0, NULL, 0);
    }
}

static
inline
CLOCK_SERVER_DELAY_REQUEST*
ClockServerpGetDelayStatus
    (
        RT_LINKED_LIST_NODE* delayRequestNode
    )
{
    return ((CLOCK_SERVER_DELAY_REQUEST*) delayRequestNode->data);
}

static
VOID
ClockServerpUnblockAllDelayedTasks
    (
        VOID
    )
{
    RT_LINKED_LIST_NODE* head;
    while (RT_SUCCESS(RtLinkedListPeekFront(&g_delayedTasks, &head)
        && ClockServerpGetDelayStatus(head)->delayUntilTick <= g_ticks))
    {
        Reply(ClockServerpGetDelayStatus(head)->td->taskId, NULL, 0);
        RtLinkedListPopFront(&g_delayedTasks);
    }
}

static
VOID
ClockServerpReplyNotifyTick
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
    return ClockServerpGetDelayStatus(a)->delayUntilTick
        < ClockServerpGetDelayStatus(b)->delayUntilTick;
}

static
VOID
ClockServerpDelayRequestNode
    (
        RT_LINKED_LIST_NODE* delayRequestNode
    )
{

    RT_LINKED_LIST_NODE* current;

    RtLinkedListPeekFront(&g_delayedTasks, &current);

    while (current != NULL && ClockServerpIsDelayedLess(delayRequestNode, current))
    {
        current = current->next;
    }

    RT_STATUS status = STATUS_FAILURE;

    if (current == NULL)
    {
        // No other task is delayed less (or there are no tasks)
        status = RtLinkedListPushBack(&g_delayedTasks, delayRequestNode);
    }
    else
    {
        status = RtLinkedListInsertBetween(&g_delayedTasks,
                                           current->previous,
                                           current,
                                           delayRequestNode);
    }

    ASSERT(RT_SUCCESS(status), "Could not delay request node.");
}

static
VOID
ClockServerpDelayTask
    (
        TASK_DESCRIPTOR* td,
        UINT delayUntilTick
    )
{
    ASSERT(td->state == ReplyBlockedState, "Task to delay must be reply-blocked.");

    CLOCK_SERVER_DELAY_REQUEST* delayRequest = &g_delayRequests[td->taskId % NUM_TASK_DESCRIPTORS];
    delayRequest->td = td;
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
            case NotifyTickRequest:
                ClockServerpReplyNotifyTick(request.td->taskId);
                break;
            case DelayRequest:
                ClockServerpDelayTask(request.td, g_ticks + request.ticks);
                break;
            case TickRequest:
                ClockServerpReplyTicks(request.td->taskId);
                break;
            case DelayUntilRequest:
                ClockServerpDelayTask(request.td, request.ticks);
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

    if (RT_SUCCESS(Send(clockServerTaskId, &request, sizeof(request), &response, sizeof(response))))
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
    CLOCK_SERVER_REQUEST request = { DelayRequest, SchedulerGetCurrentTask(), ticks };
    return ClockServerSendRequest(&request);
}

INT
Time
    (
        VOID
    )
{
    CLOCK_SERVER_REQUEST request = { TickRequest, SchedulerGetCurrentTask() };
    return ClockServerSendRequest(&request);
}

INT
DelayUntil
    (
        INT ticks
    )
{
    CLOCK_SERVER_REQUEST request = { DelayUntilRequest, SchedulerGetCurrentTask(), ticks };
    return ClockServerSendRequest(&request);
}
