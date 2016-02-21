#include "clock.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>

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

        Delay(10);
    }
}

VOID
ClockCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, ClockpTask)));
}
