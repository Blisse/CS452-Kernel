#pragma once

#include <rt.h>
#include <rtos.h>

VOID
PerformanceInit
    (
        VOID
    );

RT_STATUS
PerformanceGet
    (
        IN INT taskId,
        OUT TASK_PERFORMANCE* performance
    );

inline
VOID
PerformanceEnterTask
    (
        VOID
    );

inline
VOID
PerformanceExitTask
    (
        IN INT taskId
    );
