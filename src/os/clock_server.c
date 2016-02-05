#include "clock_server.h"

#include <bwio/bwio.h>
#include "name_server.h"

#define CLOCK_SERVER_NAME "clk"

static
VOID
ClockNotifierTask
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

VOID
ClockServerTask
    (
        VOID
    )
{
    RegisterAs(CLOCK_SERVER_NAME);
    Create(HighestPriority, ClockNotifierTask);
    Receive(NULL, NULL, 0);
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
