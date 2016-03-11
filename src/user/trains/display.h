#pragma once

#include <rt.h>
#include <user/trains.h>

VOID
DisplayCreateTask
    (
        VOID
    );

VOID
ShowKeyboardChar
    (
        IN CHAR c
    );

VOID
ShowClockTime
    (
        IN INT clockTicks
    );

VOID
ShowIdleTime
    (
        IN INT idlePercentage
    );

VOID
Log
    (
        IN STRING message,
        ...
    );

VOID
ShowSwitchDirection
    (
        IN INT idx,
        IN INT number,
        IN CHAR direction
    );

VOID
ShowSensorStatus
    (
        IN SENSOR_DATA data
    );

VOID
ShowTrainArrival
    (
        IN UCHAR train,
        IN STRING node,
        IN INT diff
    );

VOID
ShowTrainLocation
    (
        IN UCHAR train,
        IN STRING node,
        IN INT distanceToNode
    );
