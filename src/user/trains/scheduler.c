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
    StopTrainRequest,
    StartTrainRequest,
    SetTrainSpeedRequest,
} SCHEDULER_REQUEST_TYPE;

typedef struct _SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST {
    UPDATE_SCHEDULER_WORKER_REQUEST workerRequest;
} SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST;

typedef struct _SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST {
    UCHAR trainId;
    SENSOR sensor;
    UINT distancePastSensor;
} SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST;

typedef struct _SCHEDULER_TRAIN_STOP_REQUEST {
    UCHAR trainId;
} SCHEDULER_TRAIN_STOP_REQUEST;

typedef struct _SCHEDULER_TRAIN_START_REQUEST {
    UCHAR trainId;
} SCHEDULER_TRAIN_START_REQUEST;

typedef struct _SCHEDULER_TRAIN_SPEED_REQUEST {
    UCHAR trainId;
    UCHAR trainSpeed;
} SCHEDULER_TRAIN_SPEED_REQUEST;

typedef struct _SCHEDULER_REQUEST {
    SCHEDULER_REQUEST_TYPE type;

    union {
        SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST updateLocationRequest;
        SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST moveToSensorRequest;
        SCHEDULER_TRAIN_STOP_REQUEST stopRequest;
        SCHEDULER_TRAIN_START_REQUEST startRequest;
        SCHEDULER_TRAIN_SPEED_REQUEST speedRequest;
    };
} SCHEDULER_REQUEST;

typedef struct _TRAIN_SCHEDULE {
    RT_CIRCULAR_BUFFER lookaheadBuffer;
    TRACK_NODE* destinationNode;
    INT nextSpeed;
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

    VERIFY(SUCCESSFUL(Create(Priority10, SchedulerpUpdateNotifierTask)));

    TRACK_NODE* lookaheadBuffers[MAX_TRAINS][TRACK_MAX];
    TRAIN_SCHEDULE trainSchedules[MAX_TRAINS];
    RtMemset(trainSchedules, sizeof(trainSchedules), 0);

    for (UINT i = 0; i < MAX_TRAINS; i++)
    {
        RtCircularBufferInit(&trainSchedules[i].lookaheadBuffer, lookaheadBuffers[i], sizeof(lookaheadBuffers[i]));
        trainSchedules[i].nextSpeed = -1;
    }

    while (1)
    {
        INT senderId;
        SCHEDULER_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch(request.type)
        {
            case UpdateTrainDataRequest:
            {
                TRAIN_DATA trainData = request.updateLocationRequest.workerRequest.changedTrainData;
                UPDATE_SCHEDULER_WORKER_TYPE updateType = request.updateLocationRequest.workerRequest.type;
                INT trainId = trainData.trainId;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                switch (updateType)
                {
                    case UpdateSchedulerBySensor: // accurate
                    {
                        Log("%d| v:%d, a:%d", trainId, trainData.velocity, trainData.acceleration);
                        break;
                    }

                    case UpdateSchedulerByTick: // potentially inaccurate
                    {
                        break;
                    }
                }

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[trainId];
                if (trainSchedule->destinationNode != NULL)
                {
                    switch (updateType)
                    {
                        case UpdateSchedulerBySensor: // accurate
                        {
                            // update train speed at this node
                            Log("%d| v:%d, a:%d", trainId, trainData.velocity, trainData.acceleration);

                            if (trainSchedule->nextSpeed != -1)
                            {
                                VERIFY(SUCCESSFUL(TrainSetSpeed(trainId, trainSchedule->nextSpeed)));
                                trainSchedule->nextSpeed = -1;
                            }

                            // reserve 2x stopping distance of track

                            // calculate path to destination
                            break;
                        }

                        case UpdateSchedulerByTick: // potentially inaccurate
                        {

                            break;
                        }
                    }

                    // stop train at the destination
                    UINT distanceToDestination;
                    VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(trainData.currentNode, trainSchedule->destinationNode, &distanceToDestination)));
                    distanceToDestination -= trainData.distancePastCurrentNode;

                    INT stoppingDistance = distanceToAccelerate(trainData.velocity, 0, PhysicsSteadyStateDeceleration(trainId, trainData.trainSpeed));

                    Log("Stop in %d, so send stop command %d cm from %s", umToCm(stoppingDistance), umToCm(distanceToDestination), trainSchedule->destinationNode->name);

                    if (distanceToDestination < stoppingDistance)
                    {
                        VERIFY(SUCCESSFUL(TrainSetSpeed(trainId, 0)));
                        trainSchedule->destinationNode = NULL;
                        trainSchedule->nextSpeed = -1;

                        Log("Sent stop command to train %d", trainId);
                    }
                }

                // Log("Train %d at %s %3d cm", trainData.trainId, trainData.currentNode->name, trainData.distancePastCurrentNode);

                break;
            }

            case StopTrainRequest:
            {
                UCHAR trainId = request.stopRequest.trainId;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[trainId];

                TRAIN_DATA* trainData;
                VERIFY(SUCCESSFUL(GetTrainData(trainId, &trainData)));

                if (trainSchedule != NULL && trainData != NULL && trainData->trainId == trainId)
                {
                    UINT stoppingDistance = distanceToAccelerate(trainData->velocity, 0, PhysicsSteadyStateDeceleration(trainId, trainData->trainSpeed));

                    TRACK_NODE* nextNode;
                    VERIFY(SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &nextNode)));

                    INT stopPosition = abs(trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode);

                    // Log("dcn %d dpc %d", trainData->distanceCurrentToNextNode, trainData->distancePastCurrentNode);

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

            case StartTrainRequest:
            {
                SCHEDULER_TRAIN_START_REQUEST startRequest = request.startRequest;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                const INT defaultTrainSpeed = 13;
                VERIFY(SUCCESSFUL(TrainSetSpeed(startRequest.trainId, defaultTrainSpeed)));
                VERIFY(SUCCESSFUL(LocationServerLookForTrain(startRequest.trainId, defaultTrainSpeed)));
                break;
            }

            case SetTrainSpeedRequest:
            {
                SCHEDULER_TRAIN_SPEED_REQUEST speedRequest = request.speedRequest;

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[speedRequest.trainId];
                if (trainSchedule != NULL)
                {
                    trainSchedule->nextSpeed = speedRequest.trainSpeed;
                }
                break;
            }

            case TrainMoveToSensorRequest:
            {
                SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST moveToSensorRequest = request.moveToSensorRequest;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                TRACK_NODE* destinationNode;
                VERIFY(SUCCESSFUL(GetSensorNode(&moveToSensorRequest.sensor, &destinationNode)));

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[moveToSensorRequest.trainId];
                trainSchedule->destinationNode = destinationNode;

                break;
            }
        }
    }
}

VOID
SchedulerCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority21, SchedulerpTask)));
}

INT
MoveTrainToSensor (
        IN UCHAR trainId,
        IN SENSOR sensor,
        IN UINT distancePastSensor
    )
{
    SCHEDULER_REQUEST request;
    request.type = TrainMoveToSensorRequest;
    request.moveToSensorRequest.trainId = trainId;
    request.moveToSensorRequest.sensor = sensor;
    request.moveToSensorRequest.distancePastSensor = distancePastSensor;

    return SchedulerpSendRequest(&request);
}

INT
StopTrain (
        IN UCHAR trainId
    )
{
    SCHEDULER_REQUEST request;
    request.type = StopTrainRequest;
    request.stopRequest.trainId = trainId;

    return SchedulerpSendRequest(&request);
}

INT
StartTrain (
        IN UCHAR trainId
    )
{
    SCHEDULER_REQUEST request;
    request.type = StartTrainRequest;
    request.startRequest.trainId = trainId;

    return SchedulerpSendRequest(&request);
}

INT
SetTrainSpeed (
        IN UCHAR trainId,
        IN UCHAR trainSpeed
    )
{
    SCHEDULER_REQUEST request;
    request.type = SetTrainSpeedRequest;
    request.speedRequest.trainId = trainId;
    request.speedRequest.trainSpeed = trainSpeed;

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
    UPDATE_SCHEDULER_WORKER_REQUEST request;
    request.type = UpdateSchedulerBySensor;
    request.changedTrainData = *trainData;
    return SchedulerpSendWorkerRequest(&request);
}

INT
UpdateOnTick(
        IN TRAIN_DATA* trainData
    )
{
    UPDATE_SCHEDULER_WORKER_REQUEST request;
    request.type = UpdateSchedulerByTick;
    request.changedTrainData = *trainData;
    return SchedulerpSendWorkerRequest(&request);
}
