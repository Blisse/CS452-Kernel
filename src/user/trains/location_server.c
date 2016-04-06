#include "location_server.h"

#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rtosc/math.h>
#include <rtosc/string.h>
#include <track/track_node.h>

#include <rtkernel.h>
#include <rtos.h>
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
    LocationServerWorkerRegisterRequest,
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

typedef struct LOCATION_SERVER_SENSOR_CORRECTIONS {
    INT values[MAX_TRAINS][15][NUM_SENSORS];
} LOCATION_SERVER_SENSOR_CORRECTIONS;

typedef struct LOCATION_SERVER_SCHEDULER_UPDATE_REQUEST {
    TRAIN_DATA* trainData;
    UINT trackedTrainsSize;
    UINT currentTick;
    BOOLEAN onTick;
    LOCATION_SERVER_SENSOR_CORRECTIONS* sensorCorrections;
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
                    if (SUCCESSFUL(GetNextSensorNode(currentNode, &nextSensorNode)))
                    {
                        currentNode = nextSensorNode;
                        if (currentNode == activatedSensorNode)
                        {
                            return trainData;
                        }
                    }
                }
            }
        }

        if (trains[0].trainId != 0 && numTrains == 0)
        {
            return &trains[0];
        }

        TRACK_NODE* underlyingQueueBuffer[TRACK_MAX];
        RT_CIRCULAR_BUFFER pathQueue;
        RtCircularBufferInit(&pathQueue, underlyingQueueBuffer, sizeof(underlyingQueueBuffer));

        for (UINT train = 0; train < numTrains; train++)
        {
            TRAIN_DATA* trainData = &trains[train];

            VERIFY(RT_SUCCESS(RtCircularBufferClear(&pathQueue)));

            if (trainData->currentNode != NULL)
            {
                TRACK_NODE* currentNode = trainData->currentNode;

                VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &currentNode, sizeof(currentNode))));

                for (UINT level = 0; level < 10; level++)
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
                        VERIFY(RT_SUCCESS(RtCircularBufferPush(&pathQueue, &nextNode, sizeof(nextNode))));
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
LocationServerpUpdateWorkerTask()
{
    INT locationServerId = MyParentTid();
    ASSERT(SUCCESSFUL(locationServerId));

    LOCATION_SERVER_REQUEST registerRequest;
    registerRequest.type = LocationServerWorkerRegisterRequest;

    while (1)
    {
        LOCATION_SERVER_SCHEDULER_UPDATE_REQUEST schedulerUpdateRequest;

        VERIFY(SUCCESSFUL(Send(locationServerId, &registerRequest, sizeof(registerRequest), &schedulerUpdateRequest, sizeof(schedulerUpdateRequest))));

        UINT currentTick = schedulerUpdateRequest.currentTick;
        LOCATION_SERVER_SENSOR_CORRECTIONS* sensorCorrections = schedulerUpdateRequest.sensorCorrections;

        if (schedulerUpdateRequest.onTick)
        {
            for (UINT i = 0; i < schedulerUpdateRequest.trackedTrainsSize; i++)
            {
                TRAIN_DATA* trainData = &schedulerUpdateRequest.trainData[i];

                if (trainData->currentNode != NULL && trainData->nextNode != NULL)
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
                            trainData->nextNodeExpectedArrivalTick = currentTick
                                + timeToTravelDistance(distanceToTravel, trainData->velocity)
                                + sensorCorrections->values[trainData->trainId][trainData->trainSpeed][trainData->nextNode->num];
                        }
                    }

                    if (trainData->accelerationType == TrainAcceleration && trainData->velocity > trainData->targetVelocity)
                    {
                        trainData->acceleration = 0;
                        trainData->accelerationType = TrainNoAcceleration;
                    }
                    else if (trainData->accelerationType == TrainDeceleration && trainData->velocity < trainData->targetVelocity)
                    {
                        trainData->acceleration = 0;
                        trainData->accelerationType = TrainNoAcceleration;

                        if (trainData->targetVelocity == 0)
                        {
                            trainData->velocity = 0;
                        }
                    }

                    trainData->lastTick = currentTick;

                    VERIFY(SUCCESSFUL(UpdateOnTick(trainData)));
                }
            }
        }
        else
        {
            TRAIN_DATA* trainData = schedulerUpdateRequest.trainData;

            if (trainData->nextNodeExpectedArrivalTick != 0)
            {
                INT actualArrivalTimeDifference = currentTick - trainData->nextNodeExpectedArrivalTick;

                if (trainData->acceleration == 0)
                {
                    sensorCorrections->values[trainData->trainId][trainData->trainSpeed][trainData->nextNode->num] += actualArrivalTimeDifference;
                }

                ShowTrainArrival(trainData->trainId, trainData->nextNode, actualArrivalTimeDifference, distance(trainData->velocity, actualArrivalTimeDifference));
            }

            INT ticksBetweenSensors = currentTick - trainData->currentNodeArrivalTick;

            trainData->currentNode = trainData->nextNode;
            if (SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &trainData->nextNode)))
            {
                VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(trainData->currentNode, trainData->nextNode, &trainData->distanceCurrentToNextNode)));
                trainData->velocity = movingWeightedAverage(velocity(trainData->distanceCurrentToNextNode, ticksBetweenSensors), trainData->velocity, LOCATION_SERVER_ALPHA);

                trainData->distancePastCurrentNode = trainData->velocity * LOCATION_SERVER_AVERAGE_SENSOR_LATENCY;
                trainData->currentNodeArrivalTick = currentTick;

                UINT distanceToTravel = trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode;
                trainData->nextNodeExpectedArrivalTick = currentTick + timeToTravelDistance(distanceToTravel, trainData->velocity) + sensorCorrections->values[trainData->trainId][trainData->trainSpeed][trainData->nextNode->num];
            }
            else
            {
                Log("Could not find next sensor node, perhaps we're at the exit?");
                trainData->nextNode = NULL;
            }

            VERIFY(SUCCESSFUL(UpdateOnSensorNode(trainData)));
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
    RtMemset(trackedTrains, sizeof(trackedTrains), 0);
    UINT trackedTrainsSize = 0;

    LOCATION_SERVER_SENSOR_CORRECTIONS sensorCorrections;
    RtMemset(&sensorCorrections, sizeof(sensorCorrections), 0);

    INT updateWorkers[MAX_TRACKABLE_TRAINS];
    RT_CIRCULAR_BUFFER locationServerUpdateWorkers;
    RtCircularBufferInit(&locationServerUpdateWorkers, updateWorkers, sizeof(updateWorkers));

    for (UINT i = 0; i < MAX_TRACKABLE_TRAINS; i++)
    {
        VERIFY(SUCCESSFUL(Create(Priority13, LocationServerpUpdateWorkerTask)));
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
            case LocationServerWorkerRegisterRequest:
            {
                VERIFY(RT_SUCCESS(RtCircularBufferPush(&locationServerUpdateWorkers, &senderId, sizeof(senderId))));

                break;
            }

            case SensorUpdateRequest:
            {
                SENSOR* sensor = &request.sensorUpdate.sensor;

                TRACK_NODE* sensorNode;
                VERIFY(SUCCESSFUL(GetSensorNode(sensor, &sensorNode)));

                TRAIN_DATA* trainData = LocationServerpFindTrainByNextSensor(trackedTrains, trackedTrainsSize, sensorNode, 3);

                if (trainData == NULL && !RtCircularBufferIsEmpty(&lostTrains))
                {
                    trainData = &trackedTrains[trackedTrainsSize];
                    trackedTrainsSize = trackedTrainsSize + 1;

                    VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&lostTrains, trainData, sizeof(*trainData))));

                    Log("There it is! Train %d spotted by sensor %s.", trainData->trainId, sensorNode->name);
                }

                if (trainData != NULL)
                {
                    Log("I think we should attribute sensor %s to %d.", sensorNode->name, trainData->trainId);

                    if (trainData->currentNode == NULL)
                    {
                        trainData->currentNode = sensorNode;
                    }
                    else
                    {
                        trainData->nextNode = sensorNode;

                        LOCATION_SERVER_SCHEDULER_UPDsATE_REQUEST schedulerUpdateRequest;
                        schedulerUpdateRequest.trainData = trainData;
                        schedulerUpdateRequest.trackedTrainsSize = trackedTrainsSize;
                        schedulerUpdateRequest.currentTick = currentTick;
                        schedulerUpdateRequest.sensorCorrections = &sensorCorrections;
                        schedulerUpdateRequest.onTick = FALSE;

                        INT workerId = -1;
                        if (RT_SUCCESS(RtCircularBufferPeekAndPop(&locationServerUpdateWorkers, &workerId, sizeof(workerId))))
                        {
                            VERIFY(SUCCESSFUL(Reply(workerId, &schedulerUpdateRequest, sizeof(schedulerUpdateRequest))));
                        }
                        else
                        {
                            Log("All our location sensor workers are busy, sorry!");
                        }
                    }
                }
                else
                {
                    Log("We received an unexpected reading from sensor %s.", sensorNode->name);
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
                        trainData->accelerationType = TrainAcceleration;
                        trainData->targetVelocity = PhysicsSteadyStateVelocity(speedUpdate->trainId, speedUpdate->trainSpeed);
                    }
                    else if (speedUpdate->trainSpeed < trainData->trainSpeed)
                    {
                        trainData->acceleration = PhysicsSteadyStateDeceleration(speedUpdate->trainId, speedUpdate->trainSpeed);
                        trainData->accelerationType = TrainDeceleration;
                        trainData->targetVelocity = PhysicsSteadyStateVelocity(speedUpdate->trainId, speedUpdate->trainSpeed);
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
                    if (!SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &trainData->nextNode)))
                    {
                        Log("Could not reassert next node..");
                    }
                }

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }

            case TrainDirectionReverseRequest:
            {
                TRAIN_DIRECTION_REVERSE* directionReverse = &request.directionReverse;
                TRAIN_DATA* trainData = LocationServerpFindTrainById(trackedTrains, trackedTrainsSize, directionReverse->trainId);

                if (trainData != NULL)
                {
                    TRACK_NODE* tempNode = trainData->nextNode;
                    trainData->nextNode = trainData->currentNode->reverse;
                    trainData->currentNode = tempNode->reverse;

                    trainData->distancePastCurrentNode = (trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode);
                    trainData->nextNodeExpectedArrivalTick = 0;
                }

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
                newTrain.accelerationType = TrainAcceleration;
                newTrain.currentNode = NULL;
                newTrain.nextNode = NULL;
                newTrain.distancePastCurrentNode = 0;
                newTrain.distanceCurrentToNextNode = 0;
                newTrain.currentNodeArrivalTick = 0;
                newTrain.nextNodeExpectedArrivalTick = 0;
                newTrain.lastTick = currentTick;
                newTrain.targetVelocity = PhysicsSteadyStateVelocity(trainId, trainSpeed);

                VERIFY(RT_SUCCESS(RtCircularBufferPush(&lostTrains, &newTrain, sizeof(newTrain))));

                Log("Train %d should show up somewhere...", newTrain.trainId);

                break;
            }

            case TickRequest:
            {
                LOCATION_SERVER_SCHEDULER_UPDATE_REQUEST schedulerUpdateRequest;
                schedulerUpdateRequest.trainData = trackedTrains;
                schedulerUpdateRequest.trackedTrainsSize = trackedTrainsSize;
                schedulerUpdateRequest.currentTick = currentTick;
                schedulerUpdateRequest.sensorCorrections = &sensorCorrections;
                schedulerUpdateRequest.onTick = TRUE;

                INT workerId = -1;
                if (RT_SUCCESS(RtCircularBufferPeekAndPop(&locationServerUpdateWorkers, &workerId, sizeof(workerId))))
                {
                    VERIFY(SUCCESSFUL(Reply(workerId, &schedulerUpdateRequest, sizeof(schedulerUpdateRequest))));
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
