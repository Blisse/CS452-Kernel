#pragma once

#include <rt.h>

VOID
ClockServerCreateTask
    (
        VOID
    );

INT
Delay
    (
        IN INT ticks
    );

INT
Time
    (
        VOID
    );

INT
DelayUntil
    (
        IN INT ticks
    );
