#pragma once

#include <rt.h>

/************************************
 *          TRAIN API               *
 ************************************/

INT
TrainSetSpeed
    (
        IN UCHAR train,
        IN UCHAR speed
    );

INT
TrainReverse
    (
        IN UCHAR train
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
        IN UCHAR sw,
        IN SWITCH_DIRECTION direction
    );

/************************************
 *           INIT TASK              *
 ************************************/

VOID
InitTrainTasks
    (
        VOID
    );
