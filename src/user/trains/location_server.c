#include "location_server.h"

#include <rtosc/assert.h>
#include <rtosc/buffer.h>
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
} LOCATION_SERVER_REQUEST_TYPE;

typedef struct _TRAIN_SPEED_UPDATE {
    UCHAR train;
    UCHAR speed;
} TRAIN_SPEED_UPDATE;

typedef struct _TRAIN_DIRECTION_REVERSE {
    UCHAR train;
} TRAIN_DIRECTION_REVERSE;

typedef struct _SENSOR_UPDATE {
    SENSOR sensor;
} SENSOR_UPDATE;

typedef struct _TRAIN_DATA_REQUEST {
    UCHAR train;
} TRAIN_DATA_REQUEST;

typedef struct _LOCATION_SERVER_REQUEST {
    LOCATION_SERVER_REQUEST_TYPE type;

    union {
        TRAIN_DATA_REQUEST trainDataRequest;
        SENSOR_UPDATE sensorUpdate;
        TRAIN_SPEED_UPDATE speedUpdate;
        TRAIN_DIRECTION_REVERSE directionReverse;
    };
} LOCATION_SERVER_REQUEST;

typedef struct _SCHEDULER_NOTIFIER_REQUEST {
    UINT changedTrain;
} SCHEDULER_NOTIFIER_REQUEST;

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
VOID
LocationServerpSchedulerNotifierTask()
{
    INT locationServerId = MyParentTid();
    ASSERT(SUCCESSFUL(locationServerId));

    while (1)
    {
        INT senderId;
        UCHAR changedTrain;
        VERIFY(SUCCESSFUL(Receive(&senderId, &changedTrain, sizeof(changedTrain))));
        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
        ASSERT(senderId == locationServerId);
        VERIFY(SUCCESSFUL(SchedulerUpdateTrainData(changedTrain)));
    }
}

static
TRAIN_DATA*
LocationServerpFindTrainById (
        IN TRAIN_DATA* trains,
        IN UINT numTrains,
        IN UCHAR train
    )
{
    for (UINT i = 0; i < numTrains; i++)
    {
        TRAIN_DATA* trainData = &trains[i];

        if (train == trainData->train)
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
    }

    return NULL;
}

static
INT
LocationServerpGetTrainExpectedArrivalTime(
        IN TRAIN_DATA* trainData,
        IN UINT currentTick
    )
{
    if (trainData->velocity > SCHEDULER_TRAIN_NOT_MOVING_THRESHOLD)
    {
        if (trainData->distanceCurrentToNextNode > trainData->distancePastCurrentNode)
        {
            UINT distanceToNextNode = trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode;
            UINT timeToNextNode = distanceToNextNode / trainData->velocity;
            return currentTick + timeToNextNode;
        }
    }

    return 0;
}

static
INT
LocationServerpGetVelocityByReferenceNodes
    (
        IN TRAIN_DATA* trainData,
        IN INT distanceBetweenSensorNodes,
        IN INT elapsedTime
    )
{
    // v = d / t
    UINT calculatedVelocity = distanceBetweenSensorNodes / elapsedTime;

    // v = (alpha * v_new) + ((1 - alpha) * v_old)
    INT newVelocityFactor = LOCATION_SERVER_ALPHA * calculatedVelocity;
    INT oldVelocityFactor = (100 - LOCATION_SERVER_ALPHA) * trainData->velocity;

    return (newVelocityFactor + oldVelocityFactor) / 100;
}

static
INT
LocationServerpGetAccelerationTime
    (
        IN INT initialVelocity,
        IN INT finalVelocity,
        IN INT acceleration
    )
{
    return (finalVelocity - initialVelocity) / acceleration;
}

static
VOID
LocationServerpOnReachedSensorNode(
    IN TRAIN_DATA* trainData,
    IN UINT sensorArrivalTick,
    IN TRACK_NODE* sensorNode,
    IN INT* sensorLatency
    )
{
    if (trainData->currentNode == NULL)
    {
        trainData->currentNode = sensorNode;
        return;
    }

    INT diff = sensorArrivalTick - trainData->nextNodeExpectedArrivalTick - *sensorLatency;
    if (trainData->nextNodeExpectedArrivalTick != 0 && abs(diff) > SCHEDULER_ALLOWABLE_ARRIVAL_THRESHOLD)
    {
        ShowTrainArrival(trainData->train, (STRING)sensorNode->name, diff);

        *sensorLatency = diff;
    }

    TRACK_NODE* nextSensorNode;
    VERIFY(SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &nextSensorNode)));

    UINT distanceBetweenSensorNodes;
    VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(trainData->currentNode, nextSensorNode, &distanceBetweenSensorNodes)));

    trainData->currentNode = nextSensorNode;

    trainData->velocity = LocationServerpGetVelocityByReferenceNodes(trainData, distanceBetweenSensorNodes, sensorArrivalTick - trainData->currentNodeArrivalTick);

    trainData->distanceCurrentToNextNode = distanceBetweenSensorNodes;
    trainData->distancePastCurrentNode = (*sensorLatency) * trainData->velocity;
    trainData->currentNodeArrivalTick = sensorArrivalTick;
    trainData->nextNodeExpectedArrivalTick = LocationServerpGetTrainExpectedArrivalTime(trainData, sensorArrivalTick);
}

static
VOID
LocationServerpOnTick(
    IN TRAIN_DATA* trainData,
    IN UINT currentTick
    )
{
    UINT elapsedTime = currentTick - trainData->lastTick;

    trainData->distancePastCurrentNode += elapsedTime * trainData->velocity;

    if (trainData->accelerateUntilTick < currentTick)
    {
        trainData->accelerateUntilTick = 0;
        trainData->acceleration = 0;
    }

    trainData->velocity += elapsedTime * trainData->acceleration;

    if (abs(trainData->velocity) < SCHEDULER_TRAIN_NOT_MOVING_THRESHOLD)
    {
        trainData->velocity = 0;
    }

    trainData->lastTick = currentTick;

    trainData->nextNodeExpectedArrivalTick = LocationServerpGetTrainExpectedArrivalTime(trainData, currentTick);

    ShowTrainLocation(trainData->train, (STRING)trainData->currentNode->name, trainData->distancePastCurrentNode);
}

static
VOID
LocationServerpTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(LOCATION_SERVER_NAME)));
    VERIFY(SUCCESSFUL(Create(Priority23, LocationServerpSensorNotifierTask)));
    INT schedulerNotifierTaskId = Create(Priority22, LocationServerpSchedulerNotifierTask);
    ASSERT(SUCCESSFUL(schedulerNotifierTaskId));
    VERIFY(SUCCESSFUL(Create(Priority21, LocationServerpTickNotifierTask)));

    INT sensorLatencies[NUM_SENSORS];
    RtMemset(sensorLatencies, sizeof(sensorLatencies), 0);

    INT underlyingSubscriberBuffer[NUM_TASKS];
    RT_CIRCULAR_BUFFER subscriberBuffer;
    RtCircularBufferInit(&subscriberBuffer, underlyingSubscriberBuffer, sizeof(underlyingSubscriberBuffer));

    TRAIN_DATA underlyingLostTrainsBuffer[MAX_TRACKABLE_TRAINS];
    RT_CIRCULAR_BUFFER lostTrains;
    RtCircularBufferInit(&lostTrains, underlyingLostTrainsBuffer, sizeof(underlyingLostTrainsBuffer));

    TRAIN_DATA trackedTrains[MAX_TRACKABLE_TRAINS];
    UINT trackedTrainsSize = 0;

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
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                SENSOR* sensor = &request.sensorUpdate.sensor;

                TRACK_NODE* sensorNode;
                VERIFY(SUCCESSFUL(GetSensorNode(sensor, &sensorNode)));

                TRAIN_DATA* trainData = LocationServerpFindTrainByNextSensor(trackedTrains, trackedTrainsSize, sensorNode, 2);

                if (trainData == NULL && !RtCircularBufferIsEmpty(&lostTrains))
                {
                    trainData = &trackedTrains[trackedTrainsSize];
                    trackedTrainsSize = trackedTrainsSize + 1;

                    VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&lostTrains, trainData, sizeof(*trainData))));

                    Log("Found train %d", trainData->train);
                }

                if (trainData != NULL)
                {
                    UINT sensorIndex = ((sensor->module - 'A') * 16) + (sensor->number - 1);

                    LocationServerpOnReachedSensorNode(trainData, currentTick, sensorNode, &sensorLatencies[sensorIndex]);
                    VERIFY(SUCCESSFUL(Send(schedulerNotifierTaskId, &trainData->train, sizeof(trainData->train), NULL, 0)));
                }
                else
                {
                    Log("Unexpected sensor %s", sensorNode->name);
                }

                break;
            }

            case TrainSpeedUpdateRequest:
            {
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                TRAIN_SPEED_UPDATE speedUpdate = request.speedUpdate;
                TRAIN_DATA* trainData = LocationServerpFindTrainById(trackedTrains, trackedTrainsSize, speedUpdate.train);

                if (trainData != NULL)
                {
                    UCHAR currentTrainSpeed;
                    VERIFY(SUCCESSFUL(TrainGetSpeed(speedUpdate.train, &currentTrainSpeed)));

                    if (speedUpdate.speed == 0)
                    {

                        trainData->acceleration = PhysicsSteadyStateDeceleration(speedUpdate.train, currentTrainSpeed);
                        INT currentVelocity = trainData->velocity;
                        INT finalVelocity = 0;

                        trainData->accelerateUntilTick = currentTick + LocationServerpGetAccelerationTime(currentVelocity, finalVelocity, trainData->acceleration);
                    }
                    else
                    {
                        if (speedUpdate.speed > currentTrainSpeed)
                        {
                            trainData->acceleration = PhysicsSteadyStateAcceleration(speedUpdate.train, speedUpdate.speed);
                            INT currentVelocity = trainData->velocity;
                            INT finalVelocity = PhysicsSteadyStateVelocity(speedUpdate.train, speedUpdate.speed);

                            trainData->accelerateUntilTick = currentTick + LocationServerpGetAccelerationTime(currentVelocity, finalVelocity, trainData->acceleration);
                        }
                        else
                        {
                            trainData->acceleration = PhysicsSteadyStateDeceleration(speedUpdate.train, speedUpdate.speed);
                            INT currentVelocity = trainData->velocity;
                            INT finalVelocity = PhysicsSteadyStateVelocity(speedUpdate.train, speedUpdate.speed);

                            trainData->accelerateUntilTick = currentTick + LocationServerpGetAccelerationTime(currentVelocity, finalVelocity, trainData->acceleration);
                        }
                    }

                }
                else
                {
                    TRAIN_DATA newTrain;

                    newTrain.train = speedUpdate.train;
                    newTrain.velocity = 0;
                    newTrain.acceleration = PhysicsSteadyStateAcceleration(speedUpdate.train, speedUpdate.speed);
                    newTrain.currentNode = NULL;
                    newTrain.distancePastCurrentNode = 0;
                    newTrain.distanceCurrentToNextNode = 0;
                    newTrain.currentNodeArrivalTick = 0;
                    newTrain.nextNodeExpectedArrivalTick = 0;
                    newTrain.lastTick = currentTick;

                    INT commandDelay = 10; // ticks

                    UINT finalVelocity = PhysicsSteadyStateVelocity(speedUpdate.train, speedUpdate.speed);
                    newTrain.accelerateUntilTick = currentTick + commandDelay + LocationServerpGetAccelerationTime(0, finalVelocity, newTrain.acceleration);

                    VERIFY(RT_SUCCESS(RtCircularBufferPush(&lostTrains, &newTrain, sizeof(newTrain))));

                    Log("Searching for train %d", newTrain.train);
                }

                break;
            }

            case SwitchUpdateRequest:
            {
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
                TRAIN_DATA* trainData = LocationServerpFindTrainById(trackedTrains, trackedTrainsSize, trainDataRequest->train);
                VERIFY(SUCCESSFUL(Reply(senderId, &trainData, sizeof(trainData))));

                break;
            }

            case TickRequest:
            {
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                for (UINT i = 0; i < trackedTrainsSize; i++)
                {
                    TRAIN_DATA* trainData = &trackedTrains[i];
                    LocationServerpOnTick(trainData, currentTick);
                }

                break;
            }
        }
    }
}

VOID
LocationServerCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority24, LocationServerpTask)));
}

static
inline
INT
LocationServerpSendRequest(
        IN LOCATION_SERVER_REQUEST* request
    )
{
    INT locationServerId = WhoIs(LOCATION_SERVER_NAME);

    return Send(locationServerId, request, sizeof(*request), NULL, 0);
}

static
inline
INT
LocationServerpSendReplyRequest(
        IN LOCATION_SERVER_REQUEST* request,
        OUT PVOID buffer,
        IN INT bufferLength
    )
{
    INT locationServerId = WhoIs(LOCATION_SERVER_NAME);

    return Send(locationServerId, request, sizeof(*request), buffer, bufferLength);
}

INT
LocationServerUpdateTrainSpeed(
        IN UCHAR train,
        IN UCHAR speed
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainSpeedUpdateRequest;
    request.speedUpdate.train = train;
    request.speedUpdate.speed = speed;

    return LocationServerpSendRequest(&request);
}

INT
LocationServerSwitchUpdated()
{
    LOCATION_SERVER_REQUEST request = { SwitchUpdateRequest };

    return LocationServerpSendRequest(&request);
}

INT
LocationServerTrainDirectionReverse(
        IN UCHAR train
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainDirectionReverseRequest;
    request.directionReverse.train = train;

    return LocationServerpSendRequest(&request);
}

INT
GetTrainData (
        IN UCHAR train,
        OUT TRAIN_DATA** data
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainDataRequest;
    request.trainDataRequest.train = train;

    return LocationServerpSendReplyRequest(&request, data, sizeof(*data));
}
