#include "performance.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>

#include "display.h"

#define NUM_PERFORMANCE_TASKS 21
#define IDLE_TASK_ID 1

VOID
PerformancepTask
    (
        VOID
    )
{
    TASK_PERFORMANCE performanceCounters[NUM_PERFORMANCE_TASKS];

    while (1)
    {
        UINT totalTime = 0;

        UINT i;
        for (i = 0; i < NUM_PERFORMANCE_TASKS; i++)
        {
            QueryPerformance(i, &performanceCounters[i]);
            totalTime += performanceCounters[i].activeTicks;
        }

        float idleTime = (performanceCounters[IDLE_TASK_ID].activeTicks / ((float) totalTime)) * 100;

        ShowIdleTime(idleTime);

        Delay(500);
    }
}

VOID
PerformanceCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, PerformancepTask)));
}
