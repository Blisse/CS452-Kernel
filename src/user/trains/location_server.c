#include "location_server.h"

#include "display.h"
#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rtkernel.h>
#include <rtos.h>
#include <track/track_node.h>
#include <user/trains.h>

#define LOCATION_SERVER_NAME "location"

#define LOCATION_SERVER_NOTIFIER_UPDATE_INTERVAL 5 // 50ms

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
    UINT distancePastCurrentNode; // in micrometers
    TRACK_NODE* nextNode;
    DIRECTION direction;
    UINT velocity; // in micrometers / tick
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
    for(UINT i = 0; i < numTrains; i++)
    {
        TRAIN_DATA* trainData = &trains[i];

        if(node == trainData->nextNode)
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
                // TODO - Update velocity and location
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

                    Log("F %d %s", trainData->train, node->name);
                }

                // Make sure we matched the sensor to a train.  If not, just ignore the sensor
                if(NULL != trainData)
                {
                    trainData->currentNode = node;
                    trainData->distancePastCurrentNode = 0;
                    VERIFY(SUCCESSFUL(TrackFindNextSensor(node, &trainData->nextNode)));

                    Log("%s -> %s", trainData->currentNode->name, trainData->nextNode->name);

                    // Did we just find this train?
                    if(0 == trainData->velocity)
                    {
                        // TODO - Assume a velocity (the train should be moving slowly)
                    }
                    else
                    {
                        // TODO - Update velocity
                    }

                    // TODO - Send updated location and velocity to coordinator
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
                    // TODO - Update velocity
                }
                else
                {
                    // TODO - Use speed to determine target velocity
                    TRAIN_DATA newTrain;
                    newTrain.train = speedUpdate.train;
                    newTrain.currentNode = NULL;
                    newTrain.distancePastCurrentNode = 0;
                    newTrain.nextNode = NULL;
                    newTrain.direction = DirectionForward;
                    newTrain.velocity = 0;

                    // We don't know where this train is yet
                    VERIFY(RT_SUCCESS(RtCircularBufferPush(&lostTrains, &newTrain, sizeof(newTrain))));

                    Log("S %d", newTrain.train);
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

                    Log("%s -> %s", trainData->currentNode->name, trainData->nextNode->name);
                }

                break;
            }

            case FlipDirectionRequest:
            {
                TRAIN_DATA* trainData = LocationServerpFindTrainById(trackedTrains, numTrackedTrains, request.train);

                if(NULL != trainData)
                {
                    if(DirectionForward == trainData->direction)
                    {
                        trainData->direction = DirectionReverse;
                    }
                    else
                    {
                        trainData->direction = DirectionForward;
                    }

                    UINT distance;
                    VERIFY(SUCCESSFUL(TrackDistanceBetween(trainData->currentNode, trainData->nextNode, &distance)));

                    TRACK_NODE* temp = trainData->currentNode;

                    trainData->currentNode = trainData->nextNode->reverse;
                    trainData->distancePastCurrentNode = distance - trainData->distancePastCurrentNode;
                    trainData->nextNode = temp->reverse;

                    Log("%s -> %s", trainData->currentNode->name, trainData->nextNode->name);
                }
                else
                {
                    Log("Unexpected reverse %d", request.train);
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
    VERIFY(SUCCESSFUL(Create(Priority21, LocationServerpTask)));
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
