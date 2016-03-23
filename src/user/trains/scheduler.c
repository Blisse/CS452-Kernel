#include "scheduler.h"

#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>
#include <user/io.h>

#include "physics.h"
#include "track_server.h"

#define SCHEDULER_SERVER_NAME "scheduler_server"
#define SCHEDULER_TICK_PENALTY_PER_BRANCH 2 // 20 ms

typedef enum _SCHEDULER_REQUEST_TYPE {
    UpdateTrainDataRequest = 0,
    TrainMoveToSensorRequest,
    TrainStopRequest,
} SCHEDULER_REQUEST_TYPE;

typedef struct _SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST {
    UCHAR train;
} SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST;

typedef struct _SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST {
    UCHAR train;
    SENSOR sensor;
    UINT distancePastSensor;
} SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST;

typedef struct _SCHEDULER_TRAIN_STOP_REQUEST {
    UCHAR train;
} SCHEDULER_TRAIN_STOP_REQUEST;

typedef struct _SCHEDULER_REQUEST {
    SCHEDULER_REQUEST_TYPE type;

    union {
        SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST updateLocationRequest;
        SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST moveToSensorRequest;
        SCHEDULER_TRAIN_STOP_REQUEST stopRequest;
    };
} SCHEDULER_REQUEST;

typedef struct _TRAIN_SCHEDULE {
    TRACK_NODE* destinationNode;
} TRAIN_SCHEDULE;

static
VOID
SchedulerpTask()
{

    VERIFY(SUCCESSFUL(RegisterAs(SCHEDULER_SERVER_NAME)));

    TRAIN_SCHEDULE trainSchedules[MAX_TRAINS];
    RtMemset(trainSchedules, sizeof(trainSchedules), 0);

    while (1)
    {
        INT senderId;
        SCHEDULER_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        Log("receive %d %d", senderId, request.type);

        // Reply immediately to unblock tasks and ensure there are no deadlocks
        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

        switch(request.type)
        {
            case UpdateTrainDataRequest:
            {
                INT currentTime = Time();
                ASSERT(SUCCESSFUL(currentTime));

                SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST* updateLocationRequest = &request.updateLocationRequest;

                UCHAR trainId = updateLocationRequest->train;

                TRAIN_DATA* trainData;
                VERIFY(SUCCESSFUL(GetTrainData(trainId, &trainData)));

                Log("Train Velocity: %d", trainData->velocity);

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[trainId];

                if (trainData != NULL && trainData->train == trainId && trainSchedule->destinationNode != NULL)
                {
                    UINT distanceToDestination;
                    VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(trainData->currentNode, trainSchedule->destinationNode, &distanceToDestination)));
                    distanceToDestination -= trainData->distancePastCurrentNode;

                    UCHAR trainSpeed;
                    VERIFY(SUCCESSFUL(TrainGetSpeed(trainId, &trainSpeed)));

                    // d = (vf^2 - vi^2) / (2a)
                    INT stoppingDistance = (trainData->velocity * trainData->velocity) / (2 * PhysicsSteadyStateDeceleration(trainId, trainSpeed));

                    Log("Looking to stop %d from %s", distanceToDestination, trainSchedule->destinationNode->name);

                    INT commandDelay = 10; // ticks
                    INT commandDelayDistance = trainData->velocity * commandDelay;

                    if (distanceToDestination < stoppingDistance + commandDelayDistance)
                    {
                        VERIFY(SUCCESSFUL(TrainSetSpeed(trainId, 0)));
                        trainSchedule->destinationNode = NULL;

                        Log("Sent stop command to train %d", trainId);
                    }

                    trainSchedule->destinationNode = NULL;
                }

                break;
            }

            case TrainStopRequest:
            {
                SCHEDULER_TRAIN_STOP_REQUEST* stopRequest = &request.stopRequest;
                UCHAR trainId = stopRequest->train;

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[trainId];

                TRAIN_DATA* trainData;
                VERIFY(SUCCESSFUL(GetTrainData(trainId, &trainData)));

                if (trainSchedule != NULL && trainData != NULL && trainData->train == trainId)
                {
                    UCHAR trainSpeed;
                    VERIFY(SUCCESSFUL(TrainGetSpeed(trainId, &trainSpeed)));

                    // d = (vf^2 - vi^2) / (2a) - implicit *2a in deceleration constants
                    INT deceleration = PhysicsSteadyStateDeceleration(trainId, trainSpeed);
                    INT stoppingDistance = (trainData->velocity * trainData->velocity) / (deceleration);

                    TRACK_NODE* nextNode;
                    VERIFY(SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &nextNode)));

                    INT stopPosition = abs(trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode);

                    VERIFY(SUCCESSFUL(TrainSetSpeed(trainId, 0)));

                    while (stopPosition < stoppingDistance)
                    {
                        TRACK_NODE* iteraterNode;
                        VERIFY(SUCCESSFUL(GetNextSensorNode(nextNode, &iteraterNode)));

                        UINT distanceBetweenNodes;
                        VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(nextNode, iteraterNode, &distanceBetweenNodes)));

                        stopPosition += (distanceBetweenNodes * 1000);

                        nextNode = iteraterNode;
                    }

                    Log("Expected to stop %d before %s", (stopPosition - stoppingDistance), nextNode->name);
                }

                break;
            }

            case TrainMoveToSensorRequest:
            {
                SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST* moveToSensorRequest = &request.moveToSensorRequest;

                TRACK_NODE* destinationNode;
                VERIFY(SUCCESSFUL(GetSensorNode(&moveToSensorRequest->sensor, &destinationNode)));

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[moveToSensorRequest->train];
                trainSchedule->destinationNode = destinationNode;

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
SchedulerCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority22, SchedulerpTask)));
}

static
INT
SchedulerpSendRequest (
        IN SCHEDULER_REQUEST* request
    )
{
    INT result = WhoIs(SCHEDULER_SERVER_NAME);

    if (SUCCESSFUL(result))
    {
        INT schedulerId = result;

        result = Send(schedulerId, request, sizeof(*request), NULL, 0);
    }

    return result;
}

INT
SchedulerUpdateTrainData (
        IN UCHAR train
    )
{
    SCHEDULER_REQUEST request;
    request.type = UpdateTrainDataRequest;
    request.updateLocationRequest.train = train;

    return SchedulerpSendRequest(&request);
}

INT
SchedulerMoveTrainToSensor (
        IN UCHAR train,
        IN SENSOR sensor,
        IN UINT distancePastSensor
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainMoveToSensorRequest;
    request.moveToSensorRequest.train = train;
    request.moveToSensorRequest.sensor = sensor;
    request.moveToSensorRequest.distancePastSensor = distancePastSensor;

    return SchedulerpSendRequest(&request);
}

INT
SchedulerStopTrain (
        IN UCHAR train
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainStopRequest;
    request.stopRequest.train = train;

    return SchedulerpSendRequest(&request);
}
