#include "performance.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>
#include <user/io.h>

#define NUM_PERFORMANCE_TASKS NUM_TASKS
#define IDLE_TASK_ID 1

VOID
PerformancepTask()
{
    TASK_PERFORMANCE performanceCounters[NUM_PERFORMANCE_TASKS];

    while (1)
    {
        UINT totalTime = 0;

        for (UINT i = 0; i < NUM_PERFORMANCE_TASKS; i++)
        {
            QueryPerformance(i, &performanceCounters[i]);
            totalTime += performanceCounters[i].activeTicks;
        }

        INT idleTime = (performanceCounters[IDLE_TASK_ID].activeTicks / ((float) totalTime)) * 10000;

        ShowIdleTime(idleTime);

        Delay(50);
    }
}

VOID
PerformanceCreateTask()
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, PerformancepTask)));
}
