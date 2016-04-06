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
#include "track_reserver.h"
#include "track_server.h"

#define DEFAULT_TRAIN_SPEED 10
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
    ScheduleTrainSpeedRequest,
} SCHEDULER_REQUEST_TYPE;

typedef struct _SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST {
    UPDATE_SCHEDULER_WORKER_REQUEST workerRequest;
} SCHEDULER_TRAIN_UPDATE_LOCATION_REQUEST;

typedef struct _SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST {
    UINT trainId;
    SENSOR sensor;
    UINT distancePastSensor;
} SCHEDULER_TRAIN_MOVE_TO_SENSOR_REQUEST;

typedef struct _SCHEDULER_TRAIN_STOP_REQUEST {
    UINT trainId;
} SCHEDULER_TRAIN_STOP_REQUEST;

typedef struct _SCHEDULER_TRAIN_START_REQUEST {
    UINT trainId;
} SCHEDULER_TRAIN_START_REQUEST;

typedef struct _SCHEDULER_TRAIN_SPEED_REQUEST {
    UINT trainId;
    UINT trainSpeed;
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
    INT destinationNodeDelta;
    RT_CIRCULAR_BUFFER destinationBuffer;
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

    VERIFY(SUCCESSFUL(Create(Priority19, SchedulerpUpdateNotifierTask)));

    TRACK_NODE* lookaheadBuffers[MAX_TRAINS][TRACK_MAX*2];
    TRACK_NODE* destinationBuffers[MAX_TRAINS][TRACK_MAX*2];
    TRAIN_SCHEDULE trainSchedules[MAX_TRAINS];
    RtMemset(trainSchedules, sizeof(trainSchedules), 0);

    for (UINT i = 0; i < MAX_TRAINS; i++)
    {
        RtCircularBufferInit(&trainSchedules[i].lookaheadBuffer, lookaheadBuffers[i], sizeof(lookaheadBuffers[i]));
        RtCircularBufferInit(&trainSchedules[i].destinationBuffer, destinationBuffers[i], sizeof(destinationBuffers[i]));
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
                TRAIN_DATA* trainData = &request.updateLocationRequest.workerRequest.changedTrainData;
                UPDATE_SCHEDULER_WORKER_TYPE updateType = request.updateLocationRequest.workerRequest.type;
                INT trainId = trainData->trainId;

                TRAIN_SCHEDULE* trainSchedule = NULL;

                if (0 <= trainId && trainId < MAX_TRAINS)
                {
                    trainSchedule = &trainSchedules[trainId];
                }

                INT stoppingDistance = distanceToAccelerate(max(PhysicsSteadyStateVelocity(trainData->trainId, trainData->trainSpeed), trainData->velocity),
                                                            0,
                                                            PhysicsSteadyStateDeceleration(trainId, trainData->trainSpeed)) * 2;

                if (trainSchedule != NULL)
                {
                    if (updateType == UpdateSchedulerBySensor)
                    {
                        // update train speed at this node

                        if (trainSchedule->nextSpeed != -1)
                        {
                            VERIFY(SUCCESSFUL(ConductorSetTrainSpeed(trainId, trainSchedule->nextSpeed)));
                            trainSchedule->nextSpeed = -1;
                        }

                        // try to set path to destination

                        if (trainSchedule->destinationNode != NULL)
                        {
                            if (SUCCESSFUL(GetPathToDestination(trainData->currentNode, trainSchedule->destinationNode, &trainSchedule->destinationBuffer)))
                            {
                                UINT pathSize = (RtCircularBufferSize(&trainSchedule->destinationBuffer) / sizeof(TRACK_NODE*)) - 1;
                                BOOLEAN success = TRUE;
                                for (UINT i = 0; i < pathSize && success; i++)
                                {
                                    TRACK_NODE* branchNode;
                                    VERIFY(RT_SUCCESS(RtCircularBufferElementAt(&trainSchedule->destinationBuffer, i, &branchNode, sizeof(branchNode))));

                                    if (branchNode->type == NODE_BRANCH)
                                    {
                                        if (branchNode->path_distance > umToMm(trainData->velocity))
                                        {
                                            BOOLEAN isFree;
                                            VERIFY(SUCCESSFUL(IsTrackFree(branchNode, trainId, &isFree)));

                                            if (isFree)
                                            {
                                                TRACK_NODE* nextNode;
                                                VERIFY(RT_SUCCESS(RtCircularBufferElementAt(&trainSchedule->destinationBuffer, i+1, &nextNode, sizeof(nextNode))));

                                                if (branchNode->edge[DIR_STRAIGHT].dest == nextNode)
                                                {
                                                    VERIFY(SUCCESSFUL(ConductorSetSwitchDirection(branchNode->num, SwitchStraight)));
                                                }
                                                else if (branchNode->edge[DIR_CURVED].dest == nextNode)
                                                {
                                                    VERIFY(SUCCESSFUL(ConductorSetSwitchDirection(branchNode->num, SwitchCurved)));
                                                }
                                            }
                                            else
                                            {
                                                success = FALSE;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                Log("Couldn't find a path!!");
                            }
                        }

                        // update lookahead buffer
                        VERIFY(SUCCESSFUL(GetNextNodesWithinDistance(trainData->currentNode, stoppingDistance + trainData->distancePastCurrentNode, &trainSchedule->lookaheadBuffer)));
                    }

                    // reserve 2x stopping distance of track
                    if (SUCCESSFUL(ReserveTrackMultiple(&trainSchedule->lookaheadBuffer, trainId)))
                    {
                        if (trainData->trainSpeed == 0 && trainSchedule->destinationNode != NULL)
                        {
                            VERIFY(SUCCESSFUL(ConductorSetTrainSpeed(trainId, DEFAULT_TRAIN_SPEED)));
                        }
                    }
                    else
                    {
                        if (trainData->trainSpeed > DEFAULT_TRAIN_SPEED)
                        {
                            VERIFY(SUCCESSFUL(ConductorSetTrainSpeed(trainId, DEFAULT_TRAIN_SPEED)));
                            VERIFY(SUCCESSFUL(TrainSendData(trainId, 68)));
                        }
                        else if (trainData->trainSpeed > 8)
                        {
                            VERIFY(SUCCESSFUL(ConductorSetTrainSpeed(trainId, 8)));
                        }
                        else if (trainData->trainSpeed > 0)
                        {
                            VERIFY(RT_SUCCESS(RtCircularBufferClear(&trainSchedule->lookaheadBuffer)));
                            VERIFY(SUCCESSFUL(ConductorReverseTrain(trainId, trainData->trainSpeed)));
                            VERIFY(SUCCESSFUL(TrainSendData(trainId, 64)));
                        }
                    }

                    // stop at destination node + delta
                    UINT lookaheadBufferSize = (RtCircularBufferSize(&trainSchedule->lookaheadBuffer) / sizeof(TRACK_NODE*));
                    for (UINT i = 0; i < lookaheadBufferSize && trainSchedule->destinationNode != NULL; i++)
                    {
                        TRACK_NODE* aheadNode;
                        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(&trainSchedule->lookaheadBuffer, i, &aheadNode, sizeof(aheadNode))));

                        if (aheadNode == trainSchedule->destinationNode || aheadNode->reverse == trainSchedule->destinationNode)
                        {
                            UINT distanceToDestination = mmToUm(aheadNode->path_distance) + cmToUm(trainSchedule->destinationNodeDelta) - cmToUm(15) - trainData->distancePastCurrentNode;

                            Log("%s + %d cm is still %d cm away, and we require %d cm to stop.",
                                trainSchedule->destinationNode->name,
                                trainSchedule->destinationNodeDelta,
                                umToCm(distanceToDestination),
                                umToCm(stoppingDistance));

                            if (distanceToDestination < stoppingDistance)
                            {
                                Log("%d is within stopping distance of %s, let's stop!", trainId, trainSchedule->destinationNode->name);

                                VERIFY(SUCCESSFUL(ConductorSetTrainSpeed(trainId, 0)));
                                trainSchedule->destinationNode = NULL;
                                trainSchedule->nextSpeed = -1;
                            }
                        }
                    }

                    VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                    if (Time() % 6 == 0 || updateType == UpdateSchedulerBySensor)
                    {
                        ShowTrainLocation(trainData);
                    }
                }

                break;
            }

            case StopTrainRequest:
            {
                UINT trainId = request.stopRequest.trainId;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[trainId];

                TRAIN_DATA* trainData;
                VERIFY(SUCCESSFUL(GetTrainData(trainId, &trainData)));

                if (trainSchedule != NULL && trainData != NULL && trainData->trainId == trainId)
                {
                    UINT stoppingDistance = distanceToAccelerate(trainData->velocity, 0, PhysicsSteadyStateDeceleration(trainId, trainData->trainSpeed));

                    TRACK_NODE* nextNode = trainData->nextNode;

                    INT stopPosition = trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode;

                    VERIFY(SUCCESSFUL(ConductorSetTrainSpeed(trainId, 0)));

                    while (stopPosition < stoppingDistance)
                    {
                        TRACK_NODE* iteratorNode;
                        VERIFY(SUCCESSFUL(GetNextSensorNode(nextNode, &iteratorNode)));

                        UINT distanceBetweenNodes;
                        VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(nextNode, iteratorNode, &distanceBetweenNodes)));

                        stopPosition += distanceBetweenNodes;

                        nextNode = iteratorNode;
                    }

                    Log("Expected to stop %d cm after %s", umToCm(stopPosition - stoppingDistance), nextNode->name);
                }

                break;
            }

            case StartTrainRequest:
            {
                SCHEDULER_TRAIN_START_REQUEST startRequest = request.startRequest;

                VERIFY(SUCCESSFUL(ConductorSetTrainSpeed(startRequest.trainId, DEFAULT_TRAIN_SPEED)));
                VERIFY(SUCCESSFUL(LocationServerLookForTrain(startRequest.trainId, DEFAULT_TRAIN_SPEED)));
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }

            case ScheduleTrainSpeedRequest:
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

                TRACK_NODE* destinationNode;
                VERIFY(SUCCESSFUL(GetSensorNode(&moveToSensorRequest.sensor, &destinationNode)));

                TRAIN_SCHEDULE* trainSchedule = &trainSchedules[moveToSensorRequest.trainId];
                trainSchedule->destinationNode = destinationNode;
                trainSchedule->destinationNodeDelta = moveToSensorRequest.distancePastSensor;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }
        }
    }
}

VOID
SchedulerCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority24, SchedulerpTask)));
}

INT
MoveTrainToSensor (
        IN UINT trainId,
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
        IN UINT trainId
    )
{
    SCHEDULER_REQUEST request;
    request.type = StopTrainRequest;
    request.stopRequest.trainId = trainId;

    return SchedulerpSendRequest(&request);
}

INT
StartTrain (
        IN UINT trainId
    )
{
    SCHEDULER_REQUEST request;
    request.type = StartTrainRequest;
    request.startRequest.trainId = trainId;

    return SchedulerpSendRequest(&request);
}

INT
ScheduleTrainSpeed (
        IN UINT trainId,
        IN UINT trainSpeed
    )
{
    SCHEDULER_REQUEST request;
    request.type = ScheduleTrainSpeedRequest;
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
