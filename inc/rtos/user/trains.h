#pragma once

#include <rt.h>

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

/************************************
 *           SENSOR API             *
 ************************************/

#define MAX_CHANGED_SENSORS 6

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
    SENSOR_DATA sensors[MAX_CHANGED_SENSORS];
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

TRACK_NODE*
TrackFindNextSensor
    (
        IN TRACK_NODE* node, 
        IN DIRECTION direction
    );

/************************************
 *           INIT TASK              *
 ************************************/

VOID
InitTrainTasks
    (
        VOID
    );
