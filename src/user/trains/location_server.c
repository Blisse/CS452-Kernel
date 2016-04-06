#include "location_server.h"

#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rtosc/math.h>
#include <rtosc/string.h>
#include <track/track_node.h>

#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>
#include <user/io.h>

#include "physics.h"
#include "scheduler.h"
#include "track_server.h"

#define LOCATION_SERVER_NAME "location"
#define LOCATION_SERVER_NOTIFIER_UPDATE_INTERVAL 2 // 20 ms
#define LOCATION_SERVER_ALPHA 5
#define LOCATION_SERVER_AVERAGE_SENSOR_LATENCY 7 // 70 ms

// Debug builds are slower than release builds
#ifdef NDEBUG
#define SCHEDULER_ALLOWABLE_ARRIVAL_THRESHOLD 5 // 50 ms
#else
#define SCHEDULER_ALLOWABLE_ARRIVAL_THRESHOLD 10 // 100 ms
#endif

#define SCHEDULER_TRAIN_NOT_MOVING_THRESHOLD 100 // 100 micrometers per tick

typedef enum _LOCATION_SERVER_REQUEST_TYPE {
    TrainDataRequest = 0,
    TickRequest,
    SensorUpdateRequest,
    SwitchUpdateRequest,
    TrainSpeedUpdateRequest,
    TrainDirectionReverseRequest,
    TrainFindRequest,
} LOCATION_SERVER_REQUEST_TYPE;

typedef struct _TRAIN_SPEED_UPDATE {
    UCHAR trainId;
    UCHAR trainSpeed;
} TRAIN_SPEED_UPDATE;

typedef struct _TRAIN_DIRECTION_REVERSE {
    UCHAR trainId;
} TRAIN_DIRECTION_REVERSE;

typedef struct _SENSOR_UPDATE {
    SENSOR sensor;
} SENSOR_UPDATE;

typedef struct _TRAIN_DATA_REQUEST {
    UCHAR trainId;
} TRAIN_DATA_REQUEST;

typedef struct _TRAIN_LOOK_REQUEST {
    UCHAR trainId;
    UCHAR trainSpeed;
} TRAIN_LOOK_REQUEST;

typedef struct _LOCATION_SERVER_REQUEST {
    LOCATION_SERVER_REQUEST_TYPE type;

    union {
        TRAIN_DATA_REQUEST trainDataRequest;
        SENSOR_UPDATE sensorUpdate;
        TRAIN_SPEED_UPDATE speedUpdate;
        TRAIN_DIRECTION_REVERSE directionReverse;
        TRAIN_LOOK_REQUEST lookForTrainRequest;
    };
} LOCATION_SERVER_REQUEST;

typedef struct LOCATION_SERVER_SCHEDULER_UPDATE_REQUEST {
    TRAIN_DATA* trainData;
    UINT currentTick;
    BOOLEAN onTick;
} LOCATION_SERVER_SCHEDULER_UPDATE_REQUEST;

static
VOID
LocationServerpSensorNotifierTask()
{
    INT locationServerId = MyParentTid();
    ASSERT(SUCCESSFUL(locationServerId));

    LOCATION_SERVER_REQUEST request = { SensorUpdateRequest };

    while (1)
    {
        CHANGED_SENSORS changedSensors;
        VERIFY(SUCCESSFUL(SensorAwait(&changedSensors)));

        for (UINT i = 0; i < changedSensors.size; i++)
        {
            SENSOR_DATA* changedSensor = &changedSensors.sensors[i];

            if (changedSensor->isOn)
            {
                request.sensorUpdate.sensor = changedSensor->sensor;
                VERIFY(SUCCESSFUL(Send(locationServerId, &request, sizeof(request), NULL, 0)));
            }
        }
    }
}

static
VOID
LocationServerpTickNotifierTask()
{
    INT locationServerId = MyParentTid();
    ASSERT(SUCCESSFUL(locationServerId));

    LOCATION_SERVER_REQUEST request = { TickRequest };

    while (1)
    {
        VERIFY(SUCCESSFUL(Delay(LOCATION_SERVER_NOTIFIER_UPDATE_INTERVAL)));
        VERIFY(SUCCESSFUL(Send(locationServerId, &request, sizeof(request), NULL, 0)));
    }
}

static
TRAIN_DATA*
LocationServerpFindTrainById (
        IN TRAIN_DATA* trains,
        IN UINT numTrains,
        IN UCHAR trainId
    )
{
    for (UINT i = 0; i < numTrains; i++)
    {
        TRAIN_DATA* trainData = &trains[i];

        if (trainId == trainData->trainId)
        {
            return trainData;
        }
    }

    return NULL;
}

static
TRAIN_DATA*
LocationServerpFindTrainByNextSensor (
        IN TRAIN_DATA* trains,
        IN UINT numTrains,
        IN TRACK_NODE* activatedSensorNode,
        IN UINT lookaheadDepth
    )
{
    for (UINT depth = 1; depth < lookaheadDepth; depth++)
    {
        for (UINT train = 0; train < numTrains; train++)
        {
            TRAIN_DATA* trainData = &trains[train];

            if (trainData->currentNode != NULL)
            {
                TRACK_NODE* currentNode = trainData->currentNode;

                for (UINT level = 0; level < depth; level++)
                {
                    TRACK_NODE* nextSensorNode;
                    VERIFY(SUCCESSFUL(GetNextSensorNode(currentNode, &nextSensorNode)));
                    currentNode = nextSensorNode;
                    if (nextSensorNode == activatedSensorNode)
                    {
                        return trainData;
                    }
                }
            }
        }

        if (depth > 1 && numTrains == 1)
        {
            return &trains[0];
        }

        for (UINT train = 0; train < numTrains; train++)
        {
            TRAIN_DATA* trainData = &trains[train];

            if (trainData->currentNode != NULL)
            {
                TRACK_NODE* currentNode = trainData->currentNode;

                TRACK_NODE* underlyingQueueBuffer[TRACK_MAX];
                RT_CIRCULAR_BUFFER pathQueue;
                RtCircularBufferInit(&pathQueue, underlyingQueueBuffer, sizeof(underlyingQueueBuffer));

                VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &currentNode, sizeof(currentNode))));

                for (UINT level = 0; level < 8; level++)
                {
                    VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&pathQueue, &currentNode, sizeof(currentNode))));

                    if (currentNode == activatedSensorNode)
                    {
                        return trainData;
                    }

                    TRACK_NODE* nextNode;
                    if (currentNode->type == NODE_BRANCH)
                    {
                        nextNode = currentNode->edge[DIR_STRAIGHT].dest;
                        VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &nextNode, sizeof(nextNode))));

                        nextNode = currentNode->edge[DIR_CURVED].dest;
                        VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &currentNode->reverse, sizeof(currentNode->reverse))));
                    }
                    else if (currentNode->type == NODE_EXIT)
                    {
                        nextNode = currentNode->reverse;
                        VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &nextNode, sizeof(nextNode))));
                    }
                    else
                    {
                        nextNode = currentNode->edge[DIR_AHEAD].dest;
                        VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &nextNode, sizeof(nextNode))));
                    }
                }
            }
        }
    }

    return NULL;
}

static
VOID
LocationServerpSchedulerUpdateWorkerTask()
{
    while (1)
    {
        INT senderId;
        LOCATION_SERVER_SCHEDULER_UPDATE_REQUEST schedulerUpdateRequest;
        VERIFY(SUCCESSFUL(Receive(&senderId, &schedulerUpdateRequest, sizeof(schedulerUpdateRequest))));
        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

        TRAIN_DATA* trainData = schedulerUpdateRequest.trainData;
        UINT currentTick = schedulerUpdateRequest.currentTick;

        if (schedulerUpdateRequest.onTick)
        {
            if (trainData->nextNodeExpectedArrivalTick != 0)
            {
                INT actualArrivalTimeDifference = currentTick - trainData->nextNodeExpectedArrivalTick;

                if (trainData->acceleration == 0)
                {
                    sensorCorrections[trainData->trainId][sensorIndex] += actualArrivalTimeDifference;
                }

                ShowTrainArrival(trainData->trainId, sensorNode, actualArrivalTimeDifference, distance(trainData->velocity, actualArrivalTimeDifference));
            }

            INT ticksBetweenSensors = currentTick - trainData->currentNodeArrivalTick;

            trainData->currentNode = trainData->nextNode;
            VERIFY(SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &trainData->nextNode)));

            UINT distanceBetweenSensorNodes;
            VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(trainData->currentNode, trainData->nextNode, &distanceBetweenSensorNodes)));

            trainData->distanceCurrentToNextNode = distanceBetweenSensorNodes;
            trainData->velocity = movingWeightedAverage(velocity(distanceBetweenSensorNodes, ticksBetweenSensors), trainData->velocity, LOCATION_SERVER_ALPHA);
            trainData->distancePastCurrentNode = trainData->velocity * LOCATION_SERVER_AVERAGE_SENSOR_LATENCY;
            trainData->currentNodeArrivalTick = currentTick;

            UINT distanceToTravel = trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode;
            trainData->nextNodeExpectedArrivalTick = currentTick + timeToTravelDistance(distanceToTravel, trainData->velocity) + sensorCorrections[trainData->trainId][sensorIndex];

            VERIFY(SUCCESSFUL(UpdateOnTick(schedulerUpdateRequest.trainData)));
        }
        else
        {
            UINT elapsedTicks = currentTick - trainData->lastTick;
            trainData->distancePastCurrentNode += elapsedTicks * trainData->velocity;
            trainData->velocity += elapsedTicks * trainData->acceleration;

            if (abs(trainData->velocity) < SCHEDULER_TRAIN_NOT_MOVING_THRESHOLD)
            {
                trainData->velocity = 0;
                trainData->nextNodeExpectedArrivalTick = 0;
            }
            else
            {
                INT distanceToTravel = trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode;
                if (distanceToTravel > 0)
                {
                    UINT sensorIndex = trainData->nextNode->node_index;
                    trainData->nextNodeExpectedArrivalTick = currentTick + timeToTravelDistance(distanceToTravel, trainData->velocity) + sensorCorrections[trainData->trainId][sensorIndex];
                }
            }

            if (trainData->targetVelocity > 0 && trainData->velocity > abs(trainData->targetVelocity))
            {
                trainData->acceleration = 0;
            }
            else if (trainData->targetVelocity < 0 && trainData->velocity < abs(trainData->targetVelocity))
            {
                trainData->acceleration = 0;
            }

            trainData->lastTick = currentTick;

            VERIFY(SUCCESSFUL(UpdateOnSensorNode(schedulerUpdateRequest.trainData)));
        }
    }
}

static
VOID
LocationServerpTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(LOCATION_SERVER_NAME)));

    VERIFY(SUCCESSFUL(Create(Priority23, LocationServerpSensorNotifierTask)));
    VERIFY(SUCCESSFUL(Create(Priority22, LocationServerpTickNotifierTask)));

    TRAIN_DATA underlyingLostTrainsBuffer[MAX_TRACKABLE_TRAINS];
    RT_CIRCULAR_BUFFER lostTrains;
    RtCircularBufferInit(&lostTrains, underlyingLostTrainsBuffer, sizeof(underlyingLostTrainsBuffer));

    TRAIN_DATA trackedTrains[MAX_TRACKABLE_TRAINS];
    UINT trackedTrainsSize = 0;

    INT sensorCorrections[MAX_TRAINS][NUM_SENSORS];
    RtMemset(sensorCorrections, sizeof(sensorCorrections), 0);

    UINT availableSchedulerUpdateWorker = 0;
    INT schedulerUpdateWorkers[MAX_TRACKABLE_TRAINS];
    for (UINT i = 0; i < MAX_TRACKABLE_TRAINS; i++)
    {
        schedulerUpdateWorkers[i] = Create(Priority15, LocationServerpSchedulerUpdateWorkerTask);
        ASSERT(SUCCESSFUL(schedulerUpdateWorkers[i]));
    }

    while (1)
    {
        INT senderId;
        LOCATION_SERVER_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        INT currentTick = Time();
        ASSERT(SUCCESSFUL(currentTick));

        switch(request.type)
        {
            case SensorUpdateRequest:
            {
                SENSOR* sensor = &request.sensorUpdate.sensor;

                TRACK_NODE* sensorNode;
                VERIFY(SUCCESSFUL(GetSensorNode(sensor, &sensorNode)));

                UINT sensorIndex = ((sensor->module - 'A') * 16) + (sensor->number - 1);

                TRAIN_DATA* trainData = LocationServerpFindTrainByNextSensor(trackedTrains, trackedTrainsSize, sensorNode, 3);

                if (trainData == NULL && !RtCircularBufferIsEmpty(&lostTrains))
                {
                    trainData = &trackedTrains[trackedTrainsSize];
                    trackedTrainsSize = trackedTrainsSize + 1;

                    VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&lostTrains, trainData, sizeof(*trainData))));

                    Log("Found train %d at %s", trainData->trainId, sensorNode->name);
                }

                if (trainData != NULL)
                {
                    if (trainData->currentNode == NULL)
                    {
                        trainData->currentNode = sensorNode;
                        VERIFY(SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &trainData->nextNode)));
                    }
                    else
                    {
                        LOCATION_SERVER_SCHEDULER_UPDATE_REQUEST schedulerUpdateRequest;
                        schedulerUpdateRequest.trainData = trainData;
                        schedulerUpdateRequest.currentTick = currentTick;
                        schedulerUpdateRequest.onTick = FALSE;

                        UINT workerId = schedulerUpdateWorkers[availableSchedulerUpdateWorker];
                        VERIFY(SUCCESSFUL(Send(workerId, &schedulerUpdateRequest, sizeof(schedulerUpdateRequest), NULL, 0)));
                        availableSchedulerUpdateWorker = (availableSchedulerUpdateWorker + 1) % MAX_TRACKABLE_TRAINS;
                    }
                }
                else
                {
                    Log("Unexpected sensor %s", sensorNode->name);
                }

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }

            case TrainSpeedUpdateRequest:
            {
                TRAIN_SPEED_UPDATE* speedUpdate = &request.speedUpdate;
                TRAIN_DATA* trainData = LocationServerpFindTrainById(trackedTrains, trackedTrainsSize, speedUpdate->trainId);

                if (trainData != NULL)
                {
                    if (speedUpdate->trainSpeed > trainData->trainSpeed)
                    {
                        trainData->acceleration = PhysicsSteadyStateAcceleration(speedUpdate->trainId, speedUpdate->trainSpeed);
                        trainData->targetVelocity = PhysicsSteadyStateVelocity(speedUpdate->trainId, speedUpdate->trainSpeed);
                    }
                    else if (speedUpdate->trainSpeed < trainData->trainSpeed)
                    {
                        trainData->acceleration = PhysicsSteadyStateDeceleration(speedUpdate->trainId, speedUpdate->trainSpeed);
                        trainData->targetVelocity = PhysicsSteadyStateVelocity(speedUpdate->trainId, speedUpdate->trainSpeed) * -1;
                    }

                    trainData->trainSpeed = speedUpdate->trainSpeed;
                }

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }

            case SwitchUpdateRequest:
            {
                for (UINT i = 0; i < trackedTrainsSize; i++)
                {
                    TRAIN_DATA* trainData = &trackedTrains[i];
                    VERIFY(SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &trainData->nextNode)));
                }

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }

            case TrainDirectionReverseRequest:
            {
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }

            case TrainDataRequest:
            {
                TRAIN_DATA_REQUEST* trainDataRequest = &request.trainDataRequest;
                TRAIN_DATA* trainData = LocationServerpFindTrainById(trackedTrains, trackedTrainsSize, trainDataRequest->trainId);
                VERIFY(SUCCESSFUL(Reply(senderId, &trainData, sizeof(trainData))));

                break;
            }

            case TrainFindRequest:
            {
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                TRAIN_LOOK_REQUEST* lookForTrainRequest = &request.lookForTrainRequest;

                UCHAR trainId = lookForTrainRequest->trainId;
                UCHAR trainSpeed = lookForTrainRequest->trainSpeed;

                TRAIN_DATA newTrain;

                newTrain.trainId = trainId;
                newTrain.trainSpeed = trainSpeed;
                newTrain.velocity = 0;
                newTrain.acceleration = PhysicsSteadyStateAcceleration(trainId, trainSpeed);
                newTrain.currentNode = NULL;
                newTrain.nextNode = NULL;
                newTrain.distancePastCurrentNode = 0;
                newTrain.distanceCurrentToNextNode = 0;
                newTrain.currentNodeArrivalTick = 0;
                newTrain.nextNodeExpectedArrivalTick = 0;
                newTrain.lastTick = currentTick;
                newTrain.targetVelocity = PhysicsSteadyStateVelocity(trainId, trainSpeed);

                VERIFY(RT_SUCCESS(RtCircularBufferPush(&lostTrains, &newTrain, sizeof(newTrain))));

                Log("Anticipate train %d somewhere", newTrain.trainId);

                break;
            }

            case TickRequest:
            {
                for (UINT i = 0; i < trackedTrainsSize; i++)
                {
                    TRAIN_DATA* trainData = &trackedTrains[i];

                    LOCATION_SERVER_SCHEDULER_UPDATE_REQUEST schedulerUpdateRequest;
                    schedulerUpdateRequest.trainData = trainData;
                    schedulerUpdateRequest.currentTick = currentTick;
                    schedulerUpdateRequest.onTick = TRUE;

                    UINT workerId = schedulerUpdateWorkers[availableSchedulerUpdateWorker];
                    VERIFY(SUCCESSFUL(Send(workerId, &schedulerUpdateRequest, sizeof(schedulerUpdateRequest), NULL, 0)));
                    availableSchedulerUpdateWorker = (availableSchedulerUpdateWorker + 1) % MAX_TRACKABLE_TRAINS;
                }

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }
        }
    }
}

VOID
LocationServerCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority23, LocationServerpTask)));
}

static
inline
INT
LocationServerpSendReplyRequest(
        IN LOCATION_SERVER_REQUEST* request,
        IN PVOID buffer,
        IN INT bufferLength
    )
{
    INT status = WhoIs(LOCATION_SERVER_NAME);

    if (SUCCESSFUL(status))
    {
        INT locationServerId = status;
        status = Send(locationServerId, request, sizeof(*request), buffer, bufferLength);
    }

    return status;
}

INT
LocationServerUpdateTrainSpeed(
        IN UCHAR trainId,
        IN UCHAR trainSpeed
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainSpeedUpdateRequest;
    request.speedUpdate.trainId = trainId;
    request.speedUpdate.trainSpeed = trainSpeed;

    return LocationServerpSendReplyRequest(&request, NULL, 0);
}

INT
LocationServerSwitchUpdated()
{
    LOCATION_SERVER_REQUEST request;
    request.type = SwitchUpdateRequest;

    return LocationServerpSendReplyRequest(&request, NULL, 0);
}

INT
LocationServerTrainDirectionReverse(
        IN UCHAR trainId
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainDirectionReverseRequest;
    request.directionReverse.trainId = trainId;

    return LocationServerpSendReplyRequest(&request, NULL, 0);
}

INT
LocationServerLookForTrain(
        IN UCHAR trainId,
        IN UCHAR trainSpeed
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainFindRequest;
    request.lookForTrainRequest.trainId = trainId;
    request.lookForTrainRequest.trainSpeed = trainSpeed;

    return LocationServerpSendReplyRequest(&request, NULL, 0);
}

INT
GetTrainData (
        IN UCHAR trainId,
        OUT TRAIN_DATA** data
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainDataRequest;
    request.trainDataRequest.trainId = trainId;

    return LocationServerpSendReplyRequest(&request, data, sizeof(*data));
}
