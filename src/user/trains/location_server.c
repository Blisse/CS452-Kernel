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

typedef struct _TRAIN_LOOK_REQUEST {
    UCHAR train;
    UCHAR speed;
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
        TRAIN_DATA changedTrainData;
        VERIFY(SUCCESSFUL(Receive(&senderId, &changedTrainData, sizeof(changedTrainData))));
        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
        ASSERT(senderId == locationServerId);
        VERIFY(SUCCESSFUL(SchedulerUpdateTrainData(&changedTrainData)));
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
                    if (trainData->currentNode == NULL)
                    {
                        trainData->currentNode = sensorNode;
                    }
                    else
                    {
                        INT ticksBetweenSensors = currentTick - trainData->nextNodeExpectedArrivalTick;
                        if (trainData->nextNodeExpectedArrivalTick != 0)
                        {
                            ShowTrainArrival(trainData->train, sensorNode, ticksBetweenSensors);
                        }

                        TRACK_NODE* nextSensorNode;
                        VERIFY(SUCCESSFUL(GetNextSensorNode(trainData->currentNode, &nextSensorNode)));

                        UINT distanceBetweenSensorNodes;
                        VERIFY(SUCCESSFUL(GetDistanceBetweenNodes(trainData->currentNode, nextSensorNode, &distanceBetweenSensorNodes)));

                        trainData->currentNode = nextSensorNode;
                        trainData->velocity = movingWeightedAverage(velocity(distanceBetweenSensorNodes, ticksBetweenSensors), trainData->velocity, LOCATION_SERVER_ALPHA);
                        trainData->distanceCurrentToNextNode = distanceBetweenSensorNodes;
                        trainData->distancePastCurrentNode = trainData->velocity * LOCATION_SERVER_AVERAGE_SENSOR_LATENCY;
                        trainData->currentNodeArrivalTick = currentTick;

                        UINT distanceToTravel = trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode;
                        trainData->nextNodeExpectedArrivalTick = currentTick + timeToTravelDistance(distanceToTravel, trainData->velocity);

                        VERIFY(SUCCESSFUL(Send(schedulerNotifierTaskId, trainData, sizeof(*trainData), NULL, 0)));
                    }
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

                    if (speedUpdate.speed > currentTrainSpeed)
                    {
                        trainData->acceleration = PhysicsSteadyStateAcceleration(speedUpdate.train, speedUpdate.speed);
                    }
                    else
                    {
                        trainData->acceleration = PhysicsSteadyStateDeceleration(speedUpdate.train, speedUpdate.speed);
                    }

                   trainData->targetVelocity = PhysicsSteadyStateVelocity(speedUpdate.train, speedUpdate.speed);
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

            case TrainFindRequest:
            {
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                TRAIN_LOOK_REQUEST* lookForTrainRequest = &request.lookForTrainRequest;

                UCHAR trainId = lookForTrainRequest->train;
                UCHAR trainSpeed = lookForTrainRequest->speed;

                TRAIN_DATA new_train;

                new_train.train = trainId;
                new_train.velocity = 0;
                new_train.acceleration = PhysicsSteadyStateAcceleration(trainId, trainSpeed);
                new_train.currentNode = NULL;
                new_train.distancePastCurrentNode = 0;
                new_train.distanceCurrentToNextNode = 0;
                new_train.currentNodeArrivalTick = 0;
                new_train.nextNodeExpectedArrivalTick = 0;
                new_train.lastTick = currentTick;
                new_train.targetVelocity = PhysicsSteadyStateVelocity(trainId, trainSpeed);

                VERIFY(RT_SUCCESS(RtCircularBufferPush(&lostTrains, &new_train, sizeof(new_train))));

                Log("Searching for train %d", new_train.train);

                break;
            }

            case TickRequest:
            {
                for (UINT i = 0; i < trackedTrainsSize; i++)
                {
                    TRAIN_DATA* trainData = &trackedTrains[i];

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
                        UINT distanceToTravel = trainData->distanceCurrentToNextNode - trainData->distancePastCurrentNode;
                        if (distanceToTravel > 0)
                        {
                            trainData->nextNodeExpectedArrivalTick = currentTick + timeToTravelDistance(distanceToTravel, trainData->velocity);
                        }
                    }

                    trainData->lastTick = currentTick;

                    VERIFY(SUCCESSFUL(Send(schedulerNotifierTaskId, trainData, sizeof(*trainData), NULL, 0)));

                    ShowTrainLocation(trainData->train, trainData->currentNode, trainData->distancePastCurrentNode);
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
    VERIFY(SUCCESSFUL(Create(Priority24, LocationServerpTask)));
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

    return LocationServerpSendReplyRequest(&request, NULL, 0);
}

INT
LocationServerSwitchUpdated()
{
    LOCATION_SERVER_REQUEST request = { SwitchUpdateRequest };

    return LocationServerpSendReplyRequest(&request, NULL, 0);
}

INT
LocationServerTrainDirectionReverse(
        IN UCHAR train
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainDirectionReverseRequest;
    request.directionReverse.train = train;

    return LocationServerpSendReplyRequest(&request, NULL, 0);
}

INT
LocationServerLookForTrain(
        IN UCHAR train
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = TrainFindRequest;
    request.lookForTrainRequest.train = train;

    return LocationServerpSendReplyRequest(&request, NULL, 0);
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
