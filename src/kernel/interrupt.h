#pragma once

#include "rt.h"
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
InterruptAwait
    (
        IN TASK_DESCRIPTOR* td, 
        IN INT event
    );
