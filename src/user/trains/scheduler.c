#include "scheduler.h"

#include "display.h"
#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>

#define SCHEDULER_NAME "scheduler"
#define SCHEDULER_ALLOWABLE_ARRIVAL_THRESHOLD 8 // 80 ms

typedef enum _SCHEDULER_REQUEST_TYPE
{
    UpdateLocationRequest = 0
} SCHEDULER_REQUEST_TYPE;

typedef struct _SCHEDULER_UPDATE_LOCATION_REQUEST
{
    UCHAR train;
    TRACK_NODE* currentNode;
    UINT distancePastCurrentNode;
    TRACK_NODE* nextNode;
    UINT velocity;
} SCHEDULER_UPDATE_LOCATION_REQUEST;

typedef struct _SCHEDULER_REQUEST
{
    SCHEDULER_REQUEST_TYPE type;
    SCHEDULER_UPDATE_LOCATION_REQUEST updateLocationRequest;
} SCHEDULER_REQUEST;

typedef struct _TRAIN_SCHEDULE
{
    TRACK_NODE* nextNode;
    INT nextNodeExpectedArrivalTime;
} TRAIN_SCHEDULE;

static
INT
SchedulerpCalculateTimeToNode
    (
        IN TRACK_NODE* currentNode, 
        IN UINT distancePastCurrentNode, 
        IN TRACK_NODE* targetNode, 
        IN UINT velocity, 
        OUT INT* time
    )
{
    UINT distanceBetweenNodes;
    INT result = TrackDistanceBetween(currentNode, targetNode, &distanceBetweenNodes);

    if(SUCCESSFUL(result))
    {
        *time = (distanceBetweenNodes - distancePastCurrentNode) / velocity;
    }

    return result;
}

static
VOID
SchedulerpTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(SCHEDULER_NAME)));

    TRAIN_SCHEDULE trainSchedules[MAX_TRAINS];
    RtMemset(trainSchedules, sizeof(trainSchedules), 0);

    while(1) 
    {
        INT senderId;
        SCHEDULER_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        // Reply immediately to unblock tasks and ensure there are no deadlocks
        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

        switch(request.type)
        {
            case UpdateLocationRequest:
            {
                SCHEDULER_UPDATE_LOCATION_REQUEST* updateLocationRequest = &request.updateLocationRequest;
                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[updateLocationRequest->train];

                INT currentTime = Time();
                ASSERT(SUCCESSFUL(currentTime));

                INT diff = abs(currentTime - trainSchedule->nextNodeExpectedArrivalTime);

                // Check when we expected to arrive here
                if(trainSchedule->nextNode == updateLocationRequest->currentNode &&
                   diff >= SCHEDULER_ALLOWABLE_ARRIVAL_THRESHOLD)
                {
                    if(currentTime > trainSchedule->nextNodeExpectedArrivalTime)
                    {
                        Log("Late to %s by %d", trainSchedule->nextNode->name, diff);
                    }
                    else
                    {
                        Log("Early to %s by %d", trainSchedule->nextNode->name, diff);
                    }
                }

                // TODO - What if the train is going in reverse? Longer distance between pickup and sensor

                // Calculate when we will get to the next node
                INT timeTillNextNode = 0;
                VERIFY(SUCCESSFUL(SchedulerpCalculateTimeToNode(updateLocationRequest->currentNode, 
                                                                updateLocationRequest->distancePastCurrentNode, 
                                                                updateLocationRequest->nextNode, 
                                                                updateLocationRequest->velocity, 
                                                                &timeTillNextNode)));
                ASSERT(timeTillNextNode > 0);

                // Record the next scheduled event
                trainSchedule->nextNode = updateLocationRequest->nextNode;
                trainSchedule->nextNodeExpectedArrivalTime = currentTime + timeTillNextNode;

                break;
            }

            default:
            {
                ASSERT(FALSE);
                break;
            }
        }
    }
}

VOID
SchedulerCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority22, SchedulerpTask)));
}

INT
SchedulerUpdateLocation
    (
        IN TRACK_NODE* currentNode, 
        IN UINT distancePastCurrentNode, 
        IN TRACK_NODE* nextNode, 
        IN UINT velocity
    )
{
    INT result = WhoIs(SCHEDULER_NAME);

    if(SUCCESSFUL(result))
    {
        INT schedulerId = result;
        SCHEDULER_REQUEST request;
        request.type = UpdateLocationRequest;
        request.updateLocationRequest.currentNode = currentNode;
        request.updateLocationRequest.distancePastCurrentNode = distancePastCurrentNode;
        request.updateLocationRequest.nextNode = nextNode;
        request.updateLocationRequest.velocity = velocity;

        result = Send(schedulerId, &request, sizeof(request), NULL, 0);
    }

    return result;
}
