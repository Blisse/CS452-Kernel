#include "clock_server.h"

#include <bwio/bwio.h>
#include "name_server.h"

#define CLOCK_SERVER_NAME "clk"

static
VOID
ClockNotifierpTask
    (
        VOID
    )
{
    while(1)
    {
        AwaitEvent(EVENT_CLOCK);
        bwprintf(BWCOM2, "TICK\r\n");
    }
}

static
VOID
ClockServerpTask
    (
        VOID
    )
{
    RegisterAs(CLOCK_SERVER_NAME);
    Create(HighestPriority, ClockNotifierpTask);
    Receive(NULL, NULL, 0);
}

VOID
ClockServerCreateTask
    (
        VOID
    )
{
    Create(Priority29, ClockServerpTask);
}

INT
Delay
    (
        INT ticks
    )
{
    return 0;
}

INT
Time
    (
        VOID
    )
{
    return 0;
}

INT
DelayUntil
    (
        INT ticks
    )
{
    return 0;
}
