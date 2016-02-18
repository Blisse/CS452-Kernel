#include "performance.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>

#define NUM_PERFORMANCE_TASKS 21

VOID
PerformancepTask
    (
        VOID
    )
{
    DelayUntil(1500);

    IO_DEVICE com2Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom2, &com2Device)));

    WriteString(&com2Device, "\r\n");
    WriteString(&com2Device, "--PERFORMANCE--\r\n");

    TASK_PERFORMANCE performanceCounters[NUM_PERFORMANCE_TASKS];
    UINT i;
    UINT totalTime = 0;
    for (i = 0; i < NUM_PERFORMANCE_TASKS; i++)
    {
        QueryPerformance(i, &performanceCounters[i]);
        totalTime += performanceCounters[i].activeTicks;
    }

    for (i = 0; i < NUM_PERFORMANCE_TASKS; i++)
    {
        float fraction = (performanceCounters[i].activeTicks / ((float) totalTime)) * 100;
        UINT integerPart = fraction;
        UINT decimalPart = (fraction - integerPart) * 100;

        WriteFormattedString(&com2Device, "%d\t%u.%u%u\r\n", i, integerPart, decimalPart / 10, decimalPart % 10);
    }

    Shutdown();
}

VOID
PerformanceCreateTask
    (
        VOID
    )
{
    Create(LowestUserPriority, PerformancepTask);
}
