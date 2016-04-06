#pragma once

#include <rt.h>
#include <rtkernel.h>

VOID
PerformanceInit();

RT_STATUS
PerformanceGet (
        IN INT taskId,
        OUT TASK_PERFORMANCE* performance
    );

VOID
PerformanceEnterTask();

VOID
PerformanceExitTask (
        IN INT taskId
    );
