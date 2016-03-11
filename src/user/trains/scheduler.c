#include "scheduler.h"

#include "display.h"
#include "physics.h"
#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>

#define SCHEDULER_NAME "scheduler"
#define SCHEDULER_TICK_PENALTY_PER_BRANCH 2 // 20 ms
#define SCHEDULER_TRAIN_NOT_MOVING_THRESHOLD 100 // 100 micrometers per tick

// Debug builds are slower than release builds
#ifdef NDEBUG
#define SCHEDULER_ALLOWABLE_ARRIVAL_THRESHOLD 5 // 50 ms
#else
#define SCHEDULER_ALLOWABLE_ARRIVAL_THRESHOLD 10 // 100 ms
#endif

typedef enum _SCHEDULER_REQUEST_TYPE
{
    TrainChangedNextNodeRequest = 0,
    TrainArrivedAtNextNodeRequest,
    TrainUpdateLocationRequest,
    TrainStopAtSensorRequest,
    TrainStopRequest,
} SCHEDULER_REQUEST_TYPE;

typedef struct _SCHEDULER_TRAIN_CHANGED_NEXT_NODE_REQUEST
{
    UCHAR train;
    TRACK_NODE* currentNode;
    TRACK_NODE* nextNode;
} SCHEDULER_TRAIN_CHANGED_NEXT_NODE_REQUEST;

typedef struct _SCHEDULER_TRAIN_ARRIVED_AT_NEXT_NODE_REQUEST
{
    UCHAR train;
    INT arrivalTime;
} SCHEDULER_TRAIN_ARRIVED_AT_NEXT_NODE_REQUEST;

typedef struct _SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST
{
    UCHAR train;
    UINT distancePastCurrentNode;
    UINT velocity;
} SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST;

typedef struct _SCHEDULER_TRAIN_STOP_AT_SENSOR_REQUEST
{
    UCHAR train;
    SENSOR sensor;
    UINT distancePastSensor;
} SCHEDULER_TRAIN_STOP_AT_SENSOR_REQUEST;

typedef struct _SCHEDULER_TRAIN_STOP_REQUEST
{
    UCHAR train;
} SCHEDULER_TRAIN_STOP_REQUEST;

typedef struct _SCHEDULER_REQUEST
{
    SCHEDULER_REQUEST_TYPE type;

    union
    {
        SCHEDULER_TRAIN_CHANGED_NEXT_NODE_REQUEST changedNextNodeRequest;
        SCHEDULER_TRAIN_ARRIVED_AT_NEXT_NODE_REQUEST arrivedAtNextNodeRequest;
        SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST updateLocationRequest;
        SCHEDULER_TRAIN_STOP_AT_SENSOR_REQUEST stopAtSensorRequest;
        SCHEDULER_TRAIN_STOP_REQUEST stopRequest;
    };
} SCHEDULER_REQUEST;

typedef struct _TRAIN_SCHEDULE
{
    TRACK_NODE* nextNode;
    UINT nextNodeDistance;
    INT nextNodeExpectedArrivalTime;
    TRACK_NODE* stopNode;
    BOOLEAN stopping;
    UCHAR stopSpeed;
} TRAIN_SCHEDULE;

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
            case TrainChangedNextNodeRequest:
            {
                SCHEDULER_TRAIN_CHANGED_NEXT_NODE_REQUEST* changedNextNodeRequest = &request.changedNextNodeRequest;
                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[changedNextNodeRequest->train];

                trainSchedule->nextNode = changedNextNodeRequest->nextNode;
                trainSchedule->nextNodeExpectedArrivalTime = 0;
                VERIFY(SUCCESSFUL(TrackDistanceBetween(changedNextNodeRequest->currentNode,
                                                       changedNextNodeRequest->nextNode,
                                                       &trainSchedule->nextNodeDistance)));
                break;
            }

            case TrainArrivedAtNextNodeRequest:
            {
                INT currentTime = Time();
                ASSERT(SUCCESSFUL(currentTime));

                SCHEDULER_TRAIN_ARRIVED_AT_NEXT_NODE_REQUEST* arrivedAtNextNodeRequest = &request.arrivedAtNextNodeRequest;
                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[arrivedAtNextNodeRequest->train];

                // This might be the first time we've seen this train
                if(trainSchedule->nextNodeExpectedArrivalTime > 0)
                {
                    INT diff = arrivedAtNextNodeRequest->arrivalTime - trainSchedule->nextNodeExpectedArrivalTime;

                    if (abs(diff) > SCHEDULER_ALLOWABLE_ARRIVAL_THRESHOLD || currentTime % 10 == 0)
                    {
                        ShowTrainArrival(arrivedAtNextNodeRequest->train, (STRING)trainSchedule->nextNode->name, diff);
                    }
                }

                break;
            }

            case TrainUpdateLocationRequest:
            {
                INT currentTime = Time();
                ASSERT(SUCCESSFUL(currentTime));

                SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST* updateLocationRequest = &request.updateLocationRequest;
                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[updateLocationRequest->train];

                if(updateLocationRequest->velocity > SCHEDULER_TRAIN_NOT_MOVING_THRESHOLD)
                {
                    // Due to sensor latency, we may believe we've gone past the sensor
                    // If we think we've gone past the sensor, then just use our last arrival time guess
                    if(trainSchedule->nextNodeDistance > updateLocationRequest->distancePastCurrentNode)
                    {
                        UINT remainingDistance = trainSchedule->nextNodeDistance - updateLocationRequest->distancePastCurrentNode;
                        UINT timeTillNextNode = remainingDistance / updateLocationRequest->velocity;

                        // Some sensors are sticky and take longer than others to activate
                        //UINT correctiveTime = TrackGetCorrectiveTime(trainSchedule->nextNode);

                        trainSchedule->nextNodeExpectedArrivalTime = currentTime + timeTillNextNode;// + correctiveTime;
                    }
                }
                else
                {
                    trainSchedule->nextNodeExpectedArrivalTime = 0;
                }

                if (currentTime % 10 == 0)
                {
                    ShowTrainLocation(updateLocationRequest->train, (STRING)trainSchedule->nextNode->name, (INT)trainSchedule->nextNodeDistance - (INT)updateLocationRequest->distancePastCurrentNode);
                }

                if (trainSchedule->stopNode)
                {
                    UINT distanceToNode;
                    VERIFY(SUCCESSFUL(TrackDistanceBetween(trainSchedule->nextNode, trainSchedule->stopNode, &distanceToNode)));

                    UCHAR trainSpeed;
                    VERIFY(SUCCESSFUL(TrainGetSpeed(updateLocationRequest->train, &trainSpeed)));

                    // d = (vf^2 - vi^2) / (2a)
                    INT deceleration = PhysicsSteadyStateDeceleration(updateLocationRequest->train, trainSpeed);
                    INT stoppingDistance = (updateLocationRequest->velocity * updateLocationRequest->velocity) / (deceleration);

                    INT actualDistanceToNode = distanceToNode + (trainSchedule->nextNodeDistance - updateLocationRequest->distancePastCurrentNode);
                    Log("Looking to stop at [%s] %d from [%s]", trainSchedule->nextNode->name, actualDistanceToNode, trainSchedule->stopNode->name);

                    INT sensorDelay = 5;
                    INT sensorDelayDistance = updateLocationRequest->velocity * sensorDelay;

                    if (actualDistanceToNode < stoppingDistance + sensorDelayDistance)
                    {
                        Log("Stopping %d...", updateLocationRequest->train);
                        VERIFY(SUCCESSFUL(TrainSetSpeed(updateLocationRequest->train, 0)));
                        trainSchedule->stopNode = NULL;
                    }
                }

                if (trainSchedule->stopping)
                {
                    // d = (vf^2 - vi^2) / (2a) - implicit *2 in deceleration constants
                    INT deceleration = PhysicsSteadyStateDeceleration(updateLocationRequest->train, trainSchedule->stopSpeed);
                    INT stoppingDistance = (updateLocationRequest->velocity * updateLocationRequest->velocity) / (deceleration);

                    if (trainSchedule->nextNodeDistance > stoppingDistance)
                    {
                        Log("Expected to stop %d from %s", (trainSchedule->nextNodeDistance - stoppingDistance), trainSchedule->nextNode->name);
                    }
                    else
                    {
                        TRACK_EDGE* nextEdge = TrackNextEdge(trainSchedule->nextNode);
                        TRACK_NODE* nextNode = nextEdge->dest;
                        UINT distanceToNextNode = (nextEdge->dist * 1000) + (trainSchedule->nextNodeDistance);

                        while (distanceToNextNode < stoppingDistance ||
                            (distanceToNextNode >= stoppingDistance && nextNode->type != NODE_SENSOR))
                        {
                            nextEdge = TrackNextEdge(nextNode);
                            nextNode = nextEdge->dest;
                            distanceToNextNode += (nextEdge->dist * 1000);
                        }

                        Log("Expected to stop %d from %s", distanceToNextNode, stoppingDistance, (distanceToNextNode - stoppingDistance), nextNode->name);
                    }

                    trainSchedule->stopping = FALSE;
                }

                // TODO: What if the train is going in reverse? Longer distance between pickup and sensor
                //       This doesn't really matter for a location update, but will matter for stopping distance
                break;
            }

            case TrainStopRequest:
            {
                SCHEDULER_TRAIN_STOP_REQUEST* stopRequest = &request.stopRequest;
                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[stopRequest->train];
                trainSchedule->stopping = TRUE;
                VERIFY(SUCCESSFUL(TrainGetSpeed(stopRequest->train, &trainSchedule->stopSpeed)));
                VERIFY(SUCCESSFUL(TrainSetSpeed(stopRequest->train, 0)));

                break;
            }

            case TrainStopAtSensorRequest:
            {
                SCHEDULER_TRAIN_STOP_AT_SENSOR_REQUEST* stopAtSensorRequest = &request.stopAtSensorRequest;
                TRACK_NODE* stopNode = TrackFindSensor(&stopAtSensorRequest->sensor);
                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[stopAtSensorRequest->train];
                trainSchedule->stopNode = stopNode;

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

static
INT
SchedulerpSendRequest
    (
        IN SCHEDULER_REQUEST* request
    )
{
    INT result = WhoIs(SCHEDULER_NAME);

    if(SUCCESSFUL(result))
    {
        INT schedulerId = result;

        result = Send(schedulerId, request, sizeof(*request), NULL, 0);
    }

    return result;
}

INT
SchedulerTrainChangedNextNode
    (
        IN UCHAR train,
        IN TRACK_NODE* currentNode,
        IN TRACK_NODE* nextNode
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainChangedNextNodeRequest;
    request.changedNextNodeRequest.train = train;
    request.changedNextNodeRequest.currentNode = currentNode;
    request.changedNextNodeRequest.nextNode = nextNode;

    return SchedulerpSendRequest(&request);
}

INT
SchedulerTrainArrivedAtNextNode
    (
        IN UCHAR train,
        IN INT arrivalTime
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainArrivedAtNextNodeRequest;
    request.arrivedAtNextNodeRequest.train = train;
    request.arrivedAtNextNodeRequest.arrivalTime = arrivalTime;

    return SchedulerpSendRequest(&request);
}

INT
SchedulerUpdateLocation
    (
        IN UCHAR train,
        IN UINT distancePastCurrentNode,
        IN UINT velocity
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainUpdateLocationRequest;
    request.updateLocationRequest.train = train;
    request.updateLocationRequest.distancePastCurrentNode = distancePastCurrentNode;
    request.updateLocationRequest.velocity = velocity;

    return SchedulerpSendRequest(&request);
}

INT
SchedulerStopTrainAtSensor
    (
        IN UCHAR train,
        IN SENSOR sensor
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainStopAtSensorRequest;
    request.stopAtSensorRequest.train = train;
    request.stopAtSensorRequest.sensor = sensor;
    request.stopAtSensorRequest.distancePastSensor = 0;

    return SchedulerpSendRequest(&request);
}

INT
SchedulerStopTrain
    (
        IN UCHAR train
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainStopRequest;
    request.stopRequest.train = train;

    return SchedulerpSendRequest(&request);
}
