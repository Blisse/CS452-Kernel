#pragma once

#include <rt.h>

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
ShowSwitchDirection
    (
        IN INT idx,
        IN INT number,
        IN CHAR direction
    );

VOID
ShowSensorState
    (
        IN INT idx,
        IN CHAR sensorState
    );
