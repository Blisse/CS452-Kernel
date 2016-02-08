#include "clock_server.h"

#include <rtosc/assert.h>
#include <rtosc/linked_list.h>

#include "name_server.h"

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
    RT_LINKED_LIST_NODE node;
    INT taskId;
    UINT delayUntilTick;
} CLOCK_SERVER_DELAY_REQUEST;

#define NODE_TO_DELAY_REQUEST(node) (container_of(node, CLOCK_SERVER_DELAY_REQUEST, node))

static UINT g_ticks;
static RT_LINKED_LIST g_delayedTasks;
static CLOCK_SERVER_DELAY_REQUEST g_delayRequests[NUM_TASKS];

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
VOID
ClockServerpInit
    (
        VOID
    )
{
    g_ticks = 0;
    RtLinkedListInit(&g_delayedTasks);
}

static
RT_STATUS
ClockServerpUnblockAllDelayedTasks
    (
        VOID
    )
{
    RT_STATUS status = STATUS_SUCCESS;
    RT_LINKED_LIST_NODE* node;    

    while(!RtLinkedListIsEmpty(&g_delayedTasks) && RT_SUCCESS(status))
    {
        status = RtLinkedListPeekFront(&g_delayedTasks, &node);

        if(RT_SUCCESS(status))
        {
            CLOCK_SERVER_DELAY_REQUEST* delayRequest = NODE_TO_DELAY_REQUEST(node);

            if(delayRequest->delayUntilTick <= g_ticks)
            {
                Reply(delayRequest->taskId, NULL, 0);

                status = RtLinkedListPopFront(&g_delayedTasks);
            }
            else
            {
                break;
            }
        }
    }

    return status;
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

    VERIFY(RT_SUCCESS(ClockServerpUnblockAllDelayedTasks()), "Failed to unblock delayed tasks");
}

static
RT_STATUS
ClockServerpDelayRequestNode
    (
        CLOCK_SERVER_DELAY_REQUEST* delayRequest
    )
{
    RT_LINKED_LIST_NODE* node = NULL;

    (VOID) RtLinkedListPeekFront(&g_delayedTasks, &node);

    while(NULL != node && 
          NODE_TO_DELAY_REQUEST(node)->delayUntilTick < delayRequest->delayUntilTick)
    {
        node = node->next;
    }

    if(NULL != node)
    {
        return RtLinkedListInsertBetween(&g_delayedTasks,
                                         node->previous,
                                         node,
                                         &delayRequest->node);
    }
    else
    {
        return RtLinkedListPushBack(&g_delayedTasks, &delayRequest->node);
    }
}

static
VOID
ClockServerpDelayTask
    (
        INT taskId,
        UINT delayUntilTick
    )
{
    CLOCK_SERVER_DELAY_REQUEST* delayRequest = &g_delayRequests[taskId % NUM_TASKS];

    delayRequest->taskId = taskId;
    delayRequest->delayUntilTick = delayUntilTick;

    VERIFY(RT_SUCCESS(ClockServerpDelayRequestNode(delayRequest)), "Failed to delay task");
}

static
VOID
ClockServerpReplyTicks
    (
        INT taskId
    )
{
    Reply(taskId, &g_ticks, sizeof(g_ticks));
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
INT
ClockServerSendRequest
    (
        CLOCK_SERVER_REQUEST* request
    )
{
    INT clockServerTaskId = WhoIs(CLOCK_SERVER_NAME);
    INT response;
    INT status = Send(clockServerTaskId, 
                      request, 
                      sizeof(*request), 
                      &response, 
                      sizeof(response));

    return SUCCESSFUL(status) ? response : status;
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
