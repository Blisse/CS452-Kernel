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

typedef struct _SENSOR {
    CHAR module;
    UINT number;
} SENSOR;

typedef struct _SENSOR_DATA
{
    SENSOR sensor;
    UINT status;
} SENSOR_DATA;

VOID
SensorDataRegister
    (
        VOID
    );

/************************************
 *           INIT TASK              *
 ************************************/

VOID
InitTrainTasks
    (
        VOID
    );
