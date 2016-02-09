#include "clock_server.h"

#include <rtosc/assert.h>
#include <rtosc/linked_list.h>
#include <rtos.h>

#include "courier.h"

#define CLOCK_SERVER_NAME "clk"

typedef enum _CLOCK_SERVER_REQUEST_TYPE
{
    TickRequest = 0,
    TimeRequest, 
    DelayRequest,
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

static
VOID
ClockNotifierpTask
    (
        VOID
    )
{
    CLOCK_SERVER_REQUEST notifyRequest = { TickRequest };

    CourierCreateTask(Priority29, MyTid(), MyParentTid());

    while(1)
    {
        AwaitEvent(ClockEvent);
        CourierPickup(&notifyRequest, sizeof(notifyRequest));
    }
}

static
inline
RT_STATUS
ClockServerpUnblockDelayedTasks
    (
        IN RT_LINKED_LIST* delayedTasks, 
        IN UINT currentTick
    )
{
    RT_STATUS status = STATUS_SUCCESS;
    RT_LINKED_LIST_NODE* node;    

    while(!RtLinkedListIsEmpty(delayedTasks) && RT_SUCCESS(status))
    {
        status = RtLinkedListPeekFront(delayedTasks, &node);

        if(RT_SUCCESS(status))
        {
            CLOCK_SERVER_DELAY_REQUEST* delayRequest = NODE_TO_DELAY_REQUEST(node);

            if(delayRequest->delayUntilTick <= currentTick)
            {
                Reply(delayRequest->taskId, NULL, 0);

                status = RtLinkedListPopFront(delayedTasks);
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
inline
RT_STATUS
ClockServerpDelayTask
    (
        IN RT_LINKED_LIST* delayedTasks, 
        IN CLOCK_SERVER_DELAY_REQUEST* delayRequests, 
        IN INT taskId,
        IN UINT delayUntilTick
    )
{
    CLOCK_SERVER_DELAY_REQUEST* delayRequest = &delayRequests[taskId % NUM_TASKS];
    RT_LINKED_LIST_NODE* node = NULL;

    delayRequest->taskId = taskId;
    delayRequest->delayUntilTick = delayUntilTick;    

    (VOID) RtLinkedListPeekFront(delayedTasks, &node);

    while(NULL != node && 
          NODE_TO_DELAY_REQUEST(node)->delayUntilTick < delayRequest->delayUntilTick)
    {
        node = node->next;
    }

    if(NULL != node)
    {
        return RtLinkedListInsertBetween(delayedTasks,
                                         node->previous,
                                         node,
                                         &delayRequest->node);
    }
    else
    {
        return RtLinkedListPushBack(delayedTasks, &delayRequest->node);
    }
}

static
VOID
ClockServerpTask
    (
        VOID
    )
{
    UINT currentTick = 0;
    RT_LINKED_LIST delayedTasks;
    CLOCK_SERVER_DELAY_REQUEST delayRequests[NUM_TASKS];

    RtLinkedListInit(&delayedTasks);

    VERIFY(SUCCESSFUL(RegisterAs(CLOCK_SERVER_NAME)));
    VERIFY(SUCCESSFUL(Create(HighestPriority, ClockNotifierpTask)));

    while (1)
    {
        INT taskId;
        CLOCK_SERVER_REQUEST request;

        Receive(&taskId, &request, sizeof(request));

        switch (request.type)
        {
            case TickRequest:
                Reply(taskId, NULL, 0);
                currentTick++;
                VERIFY(RT_SUCCESS(ClockServerpUnblockDelayedTasks(&delayedTasks, currentTick)));
                break;

            case TimeRequest:
                Reply(taskId, &currentTick, sizeof(currentTick));
                break;

            case DelayRequest:
                VERIFY(RT_SUCCESS(ClockServerpDelayTask(&delayedTasks, 
                                                        delayRequests, 
                                                        taskId, 
                                                        currentTick + request.ticks)));
                break;

            case DelayUntilRequest:
                if(request.ticks < currentTick)
                {
                    Reply(taskId, NULL, 0);
                }
                else
                {
                    VERIFY(RT_SUCCESS(ClockServerpDelayTask(&delayedTasks, 
                                                            delayRequests, 
                                                            taskId, 
                                                            request.ticks)));
                }
                break;

            default:
                ASSERT(FALSE);
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
    VERIFY(SUCCESSFUL(Create(Priority28, ClockServerpTask)));
}

static
INT
ClockServerSendRequest
    (
        IN CLOCK_SERVER_REQUEST* request
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
        IN INT ticks
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
        IN INT ticks
    )
{
    CLOCK_SERVER_REQUEST request = { DelayUntilRequest, ticks };
    return ClockServerSendRequest(&request);
}
