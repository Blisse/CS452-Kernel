#pragma once

#include <rt.h>
#include <track/track_data.h>
#include <rtosc/buffer.h>

typedef enum _TRACK {
    TRACK_A = 0,
    TRACK_B
} TRACK;

#define CHOSEN_TRACK TRACK_B

#define MAX_TRAINS 80
#define MAX_TRACKABLE_TRAINS 6

/************************************
 *          TRAIN API               *
 ************************************/

#define MAX_SPEED 14

typedef struct _TRAIN_DATA {
    UINT trainId; // id
    UINT trainSpeed; // 0-14

    INT velocity; // in micrometers / tick
    INT acceleration; // in micrometers / tick^2

    TRACK_NODE* currentNode;
    TRACK_NODE* nextNode;

    UINT distancePastCurrentNode; // in micrometers
    UINT distanceCurrentToNextNode; // in micrometers

    UINT currentNodeArrivalTick;
    INT nextNodeExpectedArrivalTick;

    UINT lastTick;
    INT targetVelocity;
} TRAIN_DATA;

INT
TrainSetSpeed (
        IN INT train,
        IN INT speed
    );

INT
TrainReverse (
        IN INT train
    );

INT
GetTrainData (
        IN UCHAR train,
        OUT TRAIN_DATA** data
    );

/************************************
 *          SWITCH API              *
 ************************************/

typedef enum _SWITCH_DIRECTION {
    SwitchCurved = 0,
    SwitchStraight
} SWITCH_DIRECTION;

INT
SwitchSetDirection (
        IN INT sw,
        IN SWITCH_DIRECTION direction
    );

INT
SwitchGetDirection (
        IN INT sw,
        OUT SWITCH_DIRECTION* direction
    );

/************************************
 *           SENSOR API             *
 ************************************/

#define NUM_SENSORS 80

typedef struct _SENSOR {
    CHAR module;
    UINT number;
} SENSOR;

typedef struct _SENSOR_DATA {
    SENSOR sensor;
    BOOLEAN isOn;
} SENSOR_DATA;

typedef struct _CHANGED_SENSORS {
    SENSOR_DATA sensors[MAX_TRACKABLE_TRAINS];
    UINT size;
} CHANGED_SENSORS;

INT
SensorAwait (
        OUT CHANGED_SENSORS* changedSensors
    );

/************************************
 *            TRACK API             *
 ************************************/

INT
GetSensorNode(
        IN SENSOR* sensor,
        OUT TRACK_NODE** sensorNode
    );

/************************************
 *         RESERVATION API          *
 ************************************/

INT
ReserveTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    );

INT
ReleaseTrack (
        IN TRACK_NODE* trackNode,
        IN UINT trainId
    );

/************************************
 *          SCHEDULER API           *
 ************************************/

INT
MoveTrainToSensor (
        IN UINT trainId,
        IN SENSOR sensor,
        IN UINT distancePastSensor
    );

INT
StopTrain (
        IN UINT trainId
    );

INT
StartTrain (
        IN UINT trainId
    );

INT
ScheduleTrainSpeed (
        IN UINT trainId,
        IN UINT trainSpeed
    );

/************************************
 *           INIT TASK              *
 ************************************/

VOID
InitTrainTasks();
