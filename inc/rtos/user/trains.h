#pragma once

#include <rt.h>
#include <track/track_node.h>

#define MAX_TRACKABLE_TRAINS 6

/************************************
 *          TRAIN API               *
 ************************************/

INT
TrainSetSpeed
    (
        IN INT train,
        IN INT speed
    );

INT
TrainReverse
    (
        IN INT train
    );

/************************************
 *          SWITCH API              *
 ************************************/

typedef enum _SWITCH_DIRECTION
{
    SwitchCurved = 0,
    SwitchStraight
} SWITCH_DIRECTION;

INT
SwitchSetDirection
    (
        IN INT sw,
        IN SWITCH_DIRECTION direction
    );

INT
SwitchGetDirection
    (
        IN INT sw, 
        OUT SWITCH_DIRECTION* direction
    );

/************************************
 *           SENSOR API             *
 ************************************/

typedef struct _SENSOR
{
    CHAR module;
    UINT number;
} SENSOR;

typedef struct _SENSOR_DATA
{
    SENSOR sensor;
    BOOLEAN isOn;
} SENSOR_DATA;

typedef struct _CHANGED_SENSORS
{
    SENSOR_DATA sensors[MAX_TRACKABLE_TRAINS];
    UINT size;
} CHANGED_SENSORS;

INT
SensorAwait
    (
        OUT CHANGED_SENSORS* changedSensors
    );

/************************************
 *            TRACK API             *
 ************************************/
typedef enum _TRACK
{
    TrackA = 0, 
    TrackB
} TRACK;

typedef enum _DIRECTION
{
    DirectionForward = 0, 
    DirectionReverse
} DIRECTION;

VOID
TrackInit
    (
        IN TRACK track
    );

TRACK_NODE*
TrackFindSensor
    (
        IN SENSOR* sensor
    );

INT
TrackFindNextSensor
    (
        IN TRACK_NODE* node, 
        OUT TRACK_NODE** nextSensor
    );

INT
TrackDistanceBetween
    (
        IN TRACK_NODE* n1, 
        IN TRACK_NODE* n2, 
        OUT UINT* distance
    );

/************************************
 *           INIT TASK              *
 ************************************/

VOID
InitTrainTasks
    (
        VOID
    );
