#pragma once

#include "rt.h"
#include "task_descriptor.h"

typedef enum _INTERRUPT_EVENT
{
    ClockEvent = 0, 
    NumEvent
} INTERRUPT_EVENT;

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
        IN INTERRUPT_EVENT event
    );
