#pragma once

#include <rt.h>
#include <rtkernel.h>

VOID
CourierCreateTask
    (
        IN TASK_PRIORITY priority,
        IN INT sourceTask,
        IN INT destinationTask
    );

VOID
CourierPickup
    (
        IN PVOID buffer,
        IN INT size
    );
