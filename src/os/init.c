#include <rtos.h>
#include <rtkernel.h>

#include <user/users.h>

#include <bwio/bwio.h>
#include <rtosc/assert.h>

#include "clock_server.h"
#include "idle.h"
#include "io.h"
#include "name_server.h"
#include "shutdown.h"
#include "uart.h"

#define K3_TASKS 21

static
VOID
UserPerformanceTask
    (
        VOID
    )
{
    DelayUntil(1500);

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
InitOsTasks
    (
        VOID
    )
{
    // Initialize RTOS
    IdleCreateTask();
    NameServerCreateTask();
    ShutdownCreateTask();
    ClockServerCreateTask();
    IoCreateTask();
    UartCreateTasks();

    VERIFY(RT_SUCCESS(Create(HighestUserPriority, InitUserTasks)));

    // TODO - move this to a new home
    Create(LowestUserPriority, UserPerformanceTask);
}
