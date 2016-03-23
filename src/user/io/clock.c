#include "clock.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <user/io.h>

VOID
ClockpTask()
{
    while (1) {
        INT ticks = Time();

        ShowClockTime(ticks);

        Delay(10);
    }
}

VOID
ClockCreateTask()
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, ClockpTask)));
}
