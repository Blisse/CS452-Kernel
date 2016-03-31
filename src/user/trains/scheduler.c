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
#define SCHEDULER_WORKER_NAME "scheduler_worker"

typedef enum _UPDATE_SCHEDULER_WORKER_TYPE {
    UpdateSchedulerBySensor = 0,
    UpdateSchedulerByTick,
} UPDATE_SCHEDULER_WORKER_TYPE;

typedef struct _UPDATE_SCHEDULER_WORKER_REQUEST {
    UPDATE_SCHEDULER_WORKER_TYPE type;
    TRAIN_DATA changedTrainData;
} UPDATE_SCHEDULER_WORKER_REQUEST;

typedef enum _SCHEDULER_REQUEST_TYPE {
    UpdateTrainDataRequest = 0,
    TrainMoveToSensorRequest,
    TrainStopRequest,
    TrainStartRequest,
} SCHEDULER_REQUEST_TYPE;

typedef struct _SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST {
    UPDATE_SCHEDULER_WORKER_REQUEST workerRequest;
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

static
INT
SchedulerpUpdateTrainData (
        IN UPDATE_SCHEDULER_WORKER_REQUEST* workerRequest
    )
{
    SCHEDULER_REQUEST request;
    request.type = UpdateTrainDataRequest;
    request.updateLocationRequest.workerRequest = *workerRequest;

    return SchedulerpSendRequest(&request);
}

static
VOID
SchedulerpUpdateNotifierTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(SCHEDULER_WORKER_NAME)));

    while (1)
    {
        INT senderId;
        UPDATE_SCHEDULER_WORKER_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));
        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
        VERIFY(SUCCESSFUL(SchedulerpUpdateTrainData(&request)));
    }
}

static
VOID
SchedulerpTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(SCHEDULER_SERVER_NAME)));

    VERIFY(SUCCESSFUL(Create(Priority22, SchedulerpUpdateNotifierTask)));

    TRAIN_SCHEDULE trainSchedules[MAX_TRAINS];
    RtMemset(trainSchedules, sizeof(trainSchedules), 0);

    while (1)
    {
        INT senderId;
        SCHEDULER_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch(request.type)
        {
            case UpdateTrainDataRequest:
            {
                INT currentTime = Time();
                ASSERT(SUCCESSFUL(currentTime));

                TRAIN_DATA trainData = request.updateLocationRequest.workerRequest.changedTrainData;
                INT trainId = trainData.trainId;

                Log("Train %d (v: %d, a: %d)", trainId, trainData.velocity, trainData.acceleration);

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                if (request.updateLocationRequest.workerRequest.type == UpdateSchedulerBySensor)
                {

                }

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
                UCHAR trainId = request.stopRequest.train;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[trainId];

                TRAIN_DATA* trainData;
                VERIFY(SUCCESSFUL(GetTrainData(trainId, &trainData)));

                if (trainSchedule != NULL && trainData != NULL && trainData->trainId == trainId)
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
                SCHEDULER_TRAIN_START_REQUEST startRequest = request.startRequest;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                VERIFY(SUCCESSFUL(TrainSetSpeed(startRequest.train, startRequest.speed)));
                VERIFY(SUCCESSFUL(LocationServerLookForTrain(startRequest.train)));
                break;
            }

            case TrainMoveToSensorRequest:
            {
                SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST moveToSensorRequest = request.moveToSensorRequest;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                TRACK_NODE* destinationNode;
                VERIFY(SUCCESSFUL(GetSensorNode(&moveToSensorRequest.sensor, &destinationNode)));

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[moveToSensorRequest.train];
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

static
INT
SchedulerpSendWorkerRequest (
        IN UPDATE_SCHEDULER_WORKER_REQUEST* request
    )
{
    INT result = WhoIs(SCHEDULER_WORKER_NAME);
    if (SUCCESSFUL(result))
    {
        INT schedulerWorkerId = result;
        result = Send(schedulerWorkerId, request, sizeof(*request), NULL, 0);
    }
    return result;
}

INT
UpdateOnSensorNode(
        IN TRAIN_DATA* trainData
    )
{
    UPDATE_SCHEDULER_WORKER_REQUEST request = { UpdateSchedulerBySensor, *trainData };
    return SchedulerpSendWorkerRequest(&request);
}

INT
UpdateOnTick(
        IN TRAIN_DATA* trainData
    )
{
    UPDATE_SCHEDULER_WORKER_REQUEST request = { UpdateSchedulerByTick, *trainData };
    return SchedulerpSendWorkerRequest(&request);
}
