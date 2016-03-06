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
        IN INT length
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
