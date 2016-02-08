#pragma once

#include <rt.h>
#include <rtos.h>
#include "task_descriptor.h"

VOID
InterruptInit
    (
        VOID
    );

VOID
InterruptEnable
    (
        VOID
    );

VOID
InterruptDisable
    (
        VOID
    );

RT_STATUS
InterruptAwaitEvent
    (
        IN TASK_DESCRIPTOR* td,
        IN EVENT event
    );
