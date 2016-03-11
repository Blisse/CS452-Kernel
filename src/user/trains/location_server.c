#include "location_server.h"

#include "display.h"
#include "physics.h"
#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rtkernel.h>
#include <rtos.h>
#include "scheduler.h"
#include <track/track_node.h>
#include <user/trains.h>

#define LOCATION_SERVER_NAME "location"
#define LOCATION_SERVER_NOTIFIER_UPDATE_INTERVAL 3 // 30 ms
#define LOCATION_SERVER_ALPHA 5
#define LOCATION_SERVER_AVERAGE_SENSOR_LATENCY 70 // 70 ms

typedef enum _LOCATION_SERVER_REQUEST_TYPE
{
    VelocityUpdateRequest = 0,
    SensorUpdateRequest,
    SwitchUpdatedRequest,
    SpeedUpdateRequest,
    FlipDirectionRequest
} LOCATION_SERVER_REQUEST_TYPE;

typedef struct _SPEED_UPDATE
{
    UCHAR train;
    UCHAR speed;
} SPEED_UPDATE;

typedef struct _LOCATION_SERVER_REQUEST
{
    LOCATION_SERVER_REQUEST_TYPE type;

    union
    {
        UCHAR train;
        SENSOR sensor;
        SPEED_UPDATE speedUpdate;
    };
} LOCATION_SERVER_REQUEST;

typedef struct _TRAIN_DATA
{
    UCHAR train;
    TRACK_NODE* currentNode;
    INT currentNodeArrivalTick;
    UINT distancePastCurrentNode; // in micrometers
    TRACK_NODE* nextNode;
    DIRECTION direction;
    UINT velocity; // in micrometers / tick
    INT lastTimeLocationUpdated;
} TRAIN_DATA;

static
VOID
LocationServerpVelocityNotifierTask
    (
        VOID
    )
{
    INT locationServerId = MyParentTid();
    ASSERT(SUCCESSFUL(locationServerId));

    LOCATION_SERVER_REQUEST request = { VelocityUpdateRequest };

    while(1)
    {
        VERIFY(SUCCESSFUL(Delay(LOCATION_SERVER_NOTIFIER_UPDATE_INTERVAL)));
        VERIFY(SUCCESSFUL(Send(locationServerId, &request, sizeof(request), NULL, 0)));
    }
}

static
VOID
LocationServerpSensorNotifierTask
    (
        VOID
    )
{
    INT locationServerId = MyParentTid();
    ASSERT(SUCCESSFUL(locationServerId));

    LOCATION_SERVER_REQUEST request = { SensorUpdateRequest };

    while(1)
    {
        CHANGED_SENSORS changedSensors;

        VERIFY(SUCCESSFUL(SensorAwait(&changedSensors)));

        for(UINT i = 0; i < changedSensors.size; i++)
        {
            SENSOR_DATA* changedSensor = &changedSensors.sensors[i];

            if(changedSensor->isOn)
            {
                request.sensor = changedSensor->sensor;
                VERIFY(SUCCESSFUL(Send(locationServerId, &request, sizeof(request), NULL, 0)));
            }
        }
    }
}

static
TRAIN_DATA*
LocationServerpFindTrainById
    (
        IN TRAIN_DATA* trains,
        IN UINT numTrains,
        IN UCHAR train
    )
{
    for(UINT i = 0; i < numTrains; i++)
    {
        TRAIN_DATA* trainData = &trains[i];

        if(train == trainData->train)
        {
            return trainData;
        }
    }

    return NULL;
}

static
TRAIN_DATA*
LocationServerpFindTrainByNextSensor
    (
        IN TRAIN_DATA* trains,
        IN UINT numTrains,
        IN TRACK_NODE* node
    )
{
    // Check to see if we can find the node
    for(UINT i = 0; i < numTrains; i++)
    {
        TRAIN_DATA* trainData = &trains[i];

        if(node == trainData->nextNode)
        {
            return trainData;
        }
    }

    // There are unreliable sensors on the track - check for off by one sensors
    for(UINT i = 0; i < numTrains; i++)
    {
        TRAIN_DATA* trainData = &trains[i];
        TRACK_NODE* offByOneNode;
        VERIFY(SUCCESSFUL(TrackFindNextSensor(trainData->nextNode, &offByOneNode)));

        if(node == offByOneNode)
        {
            return trainData;
        }
    }

    return NULL;
}

static
VOID
LocationServerpTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(LOCATION_SERVER_NAME)));
    VERIFY(SUCCESSFUL(Create(Priority22, LocationServerpVelocityNotifierTask)));
    VERIFY(SUCCESSFUL(Create(Priority23, LocationServerpSensorNotifierTask)));

    TRAIN_DATA underlyingLostTrainsBuffer[MAX_TRACKABLE_TRAINS];
    RT_CIRCULAR_BUFFER lostTrains;
    RtCircularBufferInit(&lostTrains, underlyingLostTrainsBuffer, sizeof(underlyingLostTrainsBuffer));

    TRAIN_DATA trackedTrains[MAX_TRACKABLE_TRAINS];
    UINT numTrackedTrains = 0;

    while(1)
    {
        INT senderId;
        LOCATION_SERVER_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        // Reply right away to unblock potentially important tasks (e.g. switch and train server)
        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

        switch(request.type)
        {
            case VelocityUpdateRequest:
            {
                INT currentTime = Time();
                ASSERT(SUCCESSFUL(currentTime));

                // Update location
                // TODO - Acceleration is not taken in to account
                for(UINT i = 0; i < numTrackedTrains; i++)
                {
                    TRAIN_DATA* trainData = &trackedTrains[i];
                    INT diff = currentTime - trainData->lastTimeLocationUpdated;

                    if(diff >= LOCATION_SERVER_NOTIFIER_UPDATE_INTERVAL)
                    {
                        trainData->distancePastCurrentNode += diff * trainData->velocity;
                        trainData->lastTimeLocationUpdated = currentTime;
                        VERIFY(SUCCESSFUL(SchedulerUpdateLocation(trainData->train, trainData->distancePastCurrentNode, trainData->velocity)));
                    }
                }

                break;
            }

            case SensorUpdateRequest:
            {
                TRACK_NODE* node = TrackFindSensor(&request.sensor);
                TRAIN_DATA* trainData = LocationServerpFindTrainByNextSensor(trackedTrains, numTrackedTrains, node);

                // If we couldn't find a train we expected to arrive at this sensor,
                // then maybe a train we're looking for tripped the sensor
                if(NULL == trainData && !RtCircularBufferIsEmpty(&lostTrains))
                {
                    trainData = &trackedTrains[numTrackedTrains];
                    numTrackedTrains = numTrackedTrains + 1;

                    VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&lostTrains, trainData, sizeof(*trainData))));

                    Log("Found %d", trainData->train);
                }

                // Make sure we matched the sensor to a train.  If not, just ignore the sensor
                if(NULL != trainData)
                {
                    INT currentTick = Time();
                    ASSERT(SUCCESSFUL(currentTick));

                    INT sensorArrivalTick = (10 * currentTick - LOCATION_SERVER_AVERAGE_SENSOR_LATENCY) / 10;

                    // Check to see if we went over a dead sensor
                    if(NULL != trainData->currentNode && node != trainData->nextNode)
                    {
                        VERIFY(SUCCESSFUL(SchedulerTrainChangedNextNode(trainData->train, trainData->currentNode, node)));
                    }

                    // Let the scheduler know we reached a sensor
                    VERIFY(SUCCESSFUL(SchedulerTrainArrivedAtNextNode(trainData->train, sensorArrivalTick)));

                    // Store old values before we overwrite them
                    TRACK_NODE* previousNode = trainData->currentNode;
                    INT previousNodeArrivalTick = trainData->currentNodeArrivalTick;

                    // Calculate the next sensor we should reach
                    trainData->currentNode = node;
                    trainData->currentNodeArrivalTick = sensorArrivalTick;
                    VERIFY(SUCCESSFUL(TrackFindNextSensor(node, &trainData->nextNode)));
                    VERIFY(SUCCESSFUL(SchedulerTrainChangedNextNode(trainData->train, trainData->currentNode, trainData->nextNode)));

                    // Update the velocity if we have a point of reference
                    if(NULL != previousNode)
                    {
                        UINT dx;
                        VERIFY(SUCCESSFUL(TrackDistanceBetween(previousNode, trainData->currentNode, &dx)));
                        UINT dt = sensorArrivalTick - previousNodeArrivalTick;
                        UINT v = dx / dt;
                        UINT newVelocityFactor = LOCATION_SERVER_ALPHA * v;
                        UINT oldVelocityFactor = (100 - LOCATION_SERVER_ALPHA) * trainData->velocity;
                        trainData->velocity = (newVelocityFactor + oldVelocityFactor) / 100;
                    }

                    // Update the location
                    trainData->distancePastCurrentNode = LOCATION_SERVER_AVERAGE_SENSOR_LATENCY * trainData->velocity / 10;
                    trainData->lastTimeLocationUpdated = currentTick;
                    VERIFY(SUCCESSFUL(SchedulerUpdateLocation(trainData->train, trainData->distancePastCurrentNode, trainData->velocity)));
                }
                else
                {
                    Log("Unexpected sensor %s", node->name);
                }

                break;
            }

            case SpeedUpdateRequest:
            {
                SPEED_UPDATE speedUpdate = request.speedUpdate;
                TRAIN_DATA* trainData = LocationServerpFindTrainById(trackedTrains, numTrackedTrains, speedUpdate.train);

                if(NULL != trainData)
                {
                    if (speedUpdate.speed != 0)
                    {
                        // TODO - This is a bad approximation
                        trainData->velocity = PhysicsSteadyStateVelocity(speedUpdate.train, speedUpdate.speed);
                        VERIFY(SUCCESSFUL(SchedulerUpdateLocation(trainData->train, trainData->distancePastCurrentNode, trainData->velocity)));
                    }
                }
                else
                {
                    TRAIN_DATA newTrain;
                    newTrain.train = speedUpdate.train;
                    newTrain.currentNode = NULL;
                    newTrain.currentNodeArrivalTick = 0;
                    newTrain.distancePastCurrentNode = 0;
                    newTrain.nextNode = NULL;
                    newTrain.direction = DirectionForward;
                    newTrain.lastTimeLocationUpdated = 0;

                    // TODO - This is a bad approximation
                    newTrain.velocity = PhysicsSteadyStateVelocity(speedUpdate.train, speedUpdate.speed);

                    // We don't know where this train is yet
                    VERIFY(RT_SUCCESS(RtCircularBufferPush(&lostTrains, &newTrain, sizeof(newTrain))));

                    Log("Looking for %d", newTrain.train);
                }

                break;
            }

            case SwitchUpdatedRequest:
            {
                // Recalculate next expected sensors
                for(UINT i = 0; i < numTrackedTrains; i++)
                {
                    TRAIN_DATA* trainData = &trackedTrains[i];

                    VERIFY(SUCCESSFUL(TrackFindNextSensor(trainData->currentNode, &trainData->nextNode)));
                    VERIFY(SUCCESSFUL(SchedulerTrainChangedNextNode(trainData->train, trainData->currentNode, trainData->nextNode)));
                    VERIFY(SUCCESSFUL(SchedulerUpdateLocation(trainData->train, trainData->distancePastCurrentNode, trainData->velocity)));
                }

                break;
            }

            case FlipDirectionRequest:
            {
                TRAIN_DATA* trainData = LocationServerpFindTrainById(trackedTrains, numTrackedTrains, request.train);

                if(NULL != trainData)
                {
                    // Figure out which direction we are now travelling
                    if(DirectionForward == trainData->direction)
                    {
                        trainData->direction = DirectionReverse;
                    }
                    else
                    {
                        trainData->direction = DirectionForward;
                    }

                    // Find the distance between our current node and the next node
                    TRACK_NODE* temp = trainData->currentNode;
                    UINT distance;
                    VERIFY(SUCCESSFUL(TrackDistanceBetween(trainData->currentNode, trainData->nextNode, &distance)));

                    // Flip the direction we are travelling
                    trainData->currentNode = trainData->nextNode->reverse;
                    trainData->distancePastCurrentNode = distance - trainData->distancePastCurrentNode;
                    trainData->nextNode = temp->reverse;

                    // Let the scheduler know to expect the new direction
                    VERIFY(SUCCESSFUL(SchedulerTrainChangedNextNode(trainData->train, trainData->currentNode, trainData->nextNode)));
                    VERIFY(SUCCESSFUL(SchedulerUpdateLocation(trainData->train, trainData->distancePastCurrentNode, trainData->velocity)));
                }

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
LocationServerCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority24, LocationServerpTask)));
}

static
inline
INT
LocationServerpSendRequest
    (
        IN LOCATION_SERVER_REQUEST* request
    )
{
    INT result = WhoIs(LOCATION_SERVER_NAME);

    if(SUCCESSFUL(result))
    {
        INT locationServerId = result;

        result = Send(locationServerId, request, sizeof(*request), NULL, 0);
    }

    return result;
}

INT
LocationServerUpdateTrainSpeed
    (
        IN UCHAR train,
        IN UCHAR speed
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = SpeedUpdateRequest;
    request.speedUpdate.train = train;
    request.speedUpdate.speed = speed;

    return LocationServerpSendRequest(&request);
}

INT
LocationServerSwitchUpdated
    (
        VOID
    )
{
    LOCATION_SERVER_REQUEST request = { SwitchUpdatedRequest };

    return LocationServerpSendRequest(&request);
}

INT
LocationServerFlipTrainDirection
    (
        IN UCHAR train
    )
{
    LOCATION_SERVER_REQUEST request;
    request.type = FlipDirectionRequest;
    request.train = train;

    return LocationServerpSendRequest(&request);
}
