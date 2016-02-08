#include "performance.h"

#include <ts7200.h>
#include "clock_server.h"
#include "task_descriptor.h"

static TASK_PERFORMANCE g_taskPerformanceCounters[NUM_TASK_DESCRIPTORS];
static UINT g_lastTick;

static
inline
VOID
PerformancepSetupTimer3
    (
        VOID
    )
{
    UINT* control = (UINT*)(TIMER3_BASE + CRTL_OFFSET);
    *control = *control | CLKSEL_MASK | MODE_MASK | ENABLE_MASK;

    *(UINT*)(TIMER3_BASE + LDR_OFFSET) = UINT_MAX;
}

static
inline
UINT
PerformancepGetTimer3
    (
        VOID
    )
{
    return *(UINT*)(TIMER3_BASE + VAL_OFFSET);
}

VOID
PerformanceInit
    (
        VOID
    )
{
    PerformancepSetupTimer3();

    UINT i;
    for (i = 0; i < NUM_TASK_DESCRIPTORS; i++)
    {
        g_taskPerformanceCounters[i].activeTicks = 0;
    }

    g_lastTick = 0;
}


RT_STATUS
PerformanceGet
    (
        IN INT taskId,
        OUT TASK_PERFORMANCE* performance
    )
{
    if (0 <= taskId && taskId < NUM_TASK_DESCRIPTORS)
    {
        *performance = g_taskPerformanceCounters[taskId];
        return STATUS_SUCCESS;
    }
    return STATUS_FAILURE;
}

inline
VOID
PerformanceEnterTask
    (
        VOID
    )
{
    g_lastTick = PerformancepGetTimer3();
}

inline
VOID
PerformanceExitTask
    (
        IN INT taskId
    )
{
    UINT timer3Value = PerformancepGetTimer3();
    if (timer3Value >= g_lastTick)
    {
        g_taskPerformanceCounters[taskId].activeTicks += (timer3Value - g_lastTick);
    }
    else
    {
        g_taskPerformanceCounters[taskId].activeTicks += (g_lastTick + (UINT_MAX - timer3Value));
    }
}
