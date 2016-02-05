#pragma once

#include "rt.h"
#include "rtos.h"

#define CLOCK_SERVER_PRIORITY Priority29

VOID
ClockServerTask
    (
        VOID
    );

INT
Delay
    (
        INT ticks
    );

INT
Time
    (
        VOID
    );

INT
DelayUntil
    (
        INT ticks
    );
