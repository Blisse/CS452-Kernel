#pragma once

#include "rt.h"
#include "rtos.h"

VOID
ClockServerCreateTask
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
