#pragma once

#include <rt.h>
#include <rtkernel.h>
#include "task_descriptor.h"

VOID
InterruptInit();

VOID
InterruptDisableAll();

RT_STATUS
InterruptAwaitEvent
    (
        IN TASK_DESCRIPTOR* td,
        IN EVENT event
    );
