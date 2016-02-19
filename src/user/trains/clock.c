#include "clock.h"

#include <rtos.h>

#include "display.h"

VOID
ClockpTask
    (
        VOID
    )
{
    while (1)
    {
        INT ticks = Time();

        ShowClockTime(ticks);

        Delay(500);
    }
}

VOID
ClockCreateTask
    (
        VOID
    )
{

}
