#pragma once

#include <rt.h>
#include <user/trains.h>

/************************************
 *           INIT TASK              *
 ************************************/

VOID
InitIoTasks();

VOID
ShowKeyboardChar (
        IN CHAR c
    );

VOID
ShowClockTime (
        IN INT clockTicks
    );

VOID
ShowIdleTime (
        IN INT idlePercentage
    );

VOID
Log (
        IN STRING message,
        ...
    );

VOID
ShowSwitchDirection (
        IN INT idx,
        IN INT number,
        IN CHAR direction
    );

VOID
ShowSensorStatus (
        IN SENSOR_DATA data
    );

VOID
ShowTrainArrival (
        IN UINT trainId,
        IN TRACK_NODE* node,
        IN INT tick,
        IN INT distance
    );

VOID
ShowTrainLocation (
        IN TRAIN_DATA* trainData
    );
