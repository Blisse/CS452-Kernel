#include "scheduler.h"

#include <rtkernel.h>
#include <rtos.h>

#include <rtosc/assert.h>
#include <rtosc/math.h>
#include <rtosc/string.h>

#include <user/trains.h>
#include <user/io.h>

#include "physics.h"
#include "location_server.h"
#include "track_server.h"

#define SCHEDULER_SERVER_NAME "scheduler_server"

typedef enum _SCHEDULER_REQUEST_TYPE {
    UpdateTrainDataRequest = 0,
    TrainMoveToSensorRequest,
    TrainStopRequest,
    TrainStartRequest,
} SCHEDULER_REQUEST_TYPE;

typedef struct _SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST {
    TRAIN_DATA* trainData;
} SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST;

typedef struct _SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST {
    UCHAR train;
    SENSOR sensor;
    UINT distancePastSensor;
} SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST;

typedef struct _SCHEDULER_TRAIN_STOP_REQUEST {
    UCHAR train;
} SCHEDULER_TRAIN_STOP_REQUEST;

typedef struct _SCHEDULER_TRAIN_START_REQUEST {
    UCHAR train;
    UCHAR speed;
} SCHEDULER_TRAIN_START_REQUEST;

typedef struct _SCHEDULER_REQUEST {
    SCHEDULER_REQUEST_TYPE type;

    union {
        SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST updateLocationRequest;
        SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST moveToSensorRequest;
        SCHEDULER_TRAIN_STOP_REQUEST stopRequest;
        SCHEDULER_TRAIN_START_REQUEST startRequest;
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

        // Reply immediately to unblock tasks and ensure there are no deadlocks
        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

        switch(request.type)
        {
            case UpdateTrainDataRequest:
            {
                INT currentTime = Time();
                ASSERT(SUCCESSFUL(currentTime));

                TRAIN_DATA trainData = *request.updateLocationRequest.trainData;
                INT trainId = trainData.train;

                Log("Train %d (v: %d, a: %d)", trainId, trainData.velocity, trainData.acceleration);

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[trainId];

                if (trainSchedule->destinationNode != NULL)
                {
                    UINT distanceToDestination;
                    VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(trainData.currentNode, trainSchedule->destinationNode, &distanceToDestination)));
                    distanceToDestination -= trainData.distancePastCurrentNode;

                    UCHAR trainSpeed;
                    VERIFY(SUCCESSFUL(TrainGetSpeed(trainId, &trainSpeed)));

                    // d = (vf^2 - vi^2) / (2a)
                    INT stoppingDistance = (trainData.velocity * trainData.velocity) / PhysicsSteadyStateDeceleration(trainId, trainSpeed);

                    Log("Stop in %d, so send stop command %d cm from %s", umToCm(stoppingDistance), umToCm(distanceToDestination), trainSchedule->destinationNode->name);

                    INT commandDelay = 5; // ticks
                    INT commandDelayDistance = trainData.velocity * commandDelay;

                    if (distanceToDestination < stoppingDistance + commandDelayDistance)
                    {
                        VERIFY(SUCCESSFUL(TrainSetSpeed(trainId, 0)));
                        trainSchedule->destinationNode = NULL;

                        Log("Sent stop command to train %d", trainId);

                        trainSchedule->destinationNode = NULL;
                    }
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
                    UINT stoppingDistance = (0 - (trainData->velocity * trainData->velocity) / deceleration) * -1;

                    TRACK_NODE* nextNode;
                    VERIFY(SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &nextNode)));

                    INT stopPosition = abs(trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode);

                    Log("dcn %d dpc %d", trainData->distanceCurrentToNextNode, trainData->distancePastCurrentNode);

                    VERIFY(SUCCESSFUL(TrainSetSpeed(trainId, 0)));

                    while (stopPosition < stoppingDistance)
                    {
                        TRACK_NODE* iteraterNode;
                        VERIFY(SUCCESSFUL(GetNextSensorNode(nextNode, &iteraterNode)));

                        UINT distanceBetweenNodes;
                        VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(nextNode, iteraterNode, &distanceBetweenNodes)));

                        stopPosition += distanceBetweenNodes;

                        nextNode = iteraterNode;
                    }

                    Log("Expected to stop %d cm before %s", umToCm(stopPosition - stoppingDistance), nextNode->name);
                }

                break;
            }

            case TrainStartRequest:
            {
                VERIFY(SUCCESSFUL(TrainSetSpeed(request.startRequest.train, request.startRequest.speed)));
                VERIFY(SUCCESSFUL(LocationServerLookForTrain(request.startRequest.train)));
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
        IN TRAIN_DATA* trainData
    )
{
    SCHEDULER_REQUEST request;
    request.type = UpdateTrainDataRequest;
    request.updateLocationRequest.trainData = trainData;

    return SchedulerpSendRequest(&request);
}

INT
MoveTrainToSensor (
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
StopTrain (
        IN UCHAR train
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainStopRequest;
    request.stopRequest.train = train;

    return SchedulerpSendRequest(&request);
}

INT
StartTrain (
        IN UCHAR train
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainStartRequest;
    request.startRequest.train = train;

    return SchedulerpSendRequest(&request);
}
