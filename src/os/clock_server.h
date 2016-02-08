#pragma once

#include <rt.h>
#include <rtos.h>

#define CLOCK_SERVER_NAME "clk"

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
