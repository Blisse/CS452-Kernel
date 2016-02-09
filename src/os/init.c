#include "init.h"

#include <bwio/bwio.h>
#include "clock_server.h"
#include "idle.h"
#include "name_server.h"
#include "uart_server.h"

#define K3_TASKS 11

static
VOID
UserPerformanceTask
    (
        VOID
    )
{
    DelayUntil(300);

    bwprintf(BWCOM2, "\r\n");
    bwprintf(BWCOM2, "--PERFORMANCE--\r\n");
    bwprintf(BWCOM2, "TASK\t%%active\r\n");

    TASK_PERFORMANCE performanceCounters[K3_TASKS];
    UINT i;
    UINT totalTime = 0;
    for (i = 0; i < K3_TASKS; i++)
    {
        QueryPerformance(i, &performanceCounters[i]);
        totalTime += performanceCounters[i].activeTicks;
    }

    for (i = 0; i < K3_TASKS; i++)
    {
        float fraction = (performanceCounters[i].activeTicks / ((float) totalTime)) * 100;
        UINT integerPart = fraction;
        UINT decimalPart = (fraction - integerPart) * 100;

        bwprintf(BWCOM2, "%d\t%u.%u%u\r\n", i, integerPart, decimalPart / 10, decimalPart % 10);
    }

    Shutdown();
}

VOID
InitTask
    (
        VOID
    )
{
    IdleCreateTask();

    NameServerCreateTask();

    ClockServerCreateTask();

    UartServerCreateTask();

    Create(Priority10, UserPerformanceTask);
}
