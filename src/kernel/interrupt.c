#include "interrupt.h"

#include <rtosc/assert.h>
#include "scheduler.h"
#include <ts7200.h>

#define TIMER_CONTROL(timerBase) ((volatile UINT*)((timerBase) + CRTL_OFFSET))
#define TIMER_LOAD(timerBase) ((volatile UINT*)((timerBase) + LDR_OFFSET))
#define TIMER_CLEAR(timerBase) ((volatile UINT*)((timerBase) + CLR_OFFSET))

#define VIC1_BASE 0x800B0000
#define STATUS_OFFSET 0
#define ENABLE_OFFSET 0x10
#define DISABLE_OFFSET 0x14

#define TC2IO_MASK 0x20

#define VIC_STATUS(vicBase) ((volatile UINT*)((vicBase) + STATUS_OFFSET))
#define VIC_INTERRUPT_ENABLE(vicBase) ((volatile UINT*)((vicBase) + ENABLE_OFFSET))
#define VIC_INTERRUPT_DISABLE(vicBase) ((volatile UINT*)((vicBase) + DISABLE_OFFSET))

extern
VOID
InterruptInstallHandler
    (
        VOID
    );

static TASK_DESCRIPTOR* g_eventHandlers[NumEvent];

static
VOID
InterruptpDisable
    (
        IN EVENT event
    )
{
    volatile UINT* vicDisable = VIC_INTERRUPT_DISABLE(VIC1_BASE);

    switch(event)
    {
        case ClockEvent:
            *vicDisable = *vicDisable | TC2IO_MASK;
            break;

        default:
            ASSERT(FALSE);
    }
} 

static
VOID
InterruptpEnable
    (
        IN EVENT event
    )
{
    volatile UINT* vicEnable = VIC_INTERRUPT_ENABLE(VIC1_BASE);
    
    switch(event)
    {
        case ClockEvent:
            *vicEnable = *vicEnable | TC2IO_MASK;
            break;

        default:
            ASSERT(FALSE);
    }
}

static
VOID
InterruptpSignalEvent
    (
        IN EVENT event
    )
{
    TASK_DESCRIPTOR* handler = g_eventHandlers[event];

    // Unblock the handler
    handler->state = ReadyState;
    TaskSetReturnValue(handler, STATUS_SUCCESS);
    SchedulerAddTask(handler);

    // Clear the handler
    g_eventHandlers[event] = NULL;
}

static
inline
VOID
InterruptpHandleClock
    (
        VOID
    )
{
    // Acknowledge the interrupt
    *TIMER_CLEAR(TIMER2_BASE) = TRUE;

    // Handle the interrupt
    InterruptpSignalEvent(ClockEvent);

    // Disable the interrupt
    InterruptpDisable(ClockEvent);
}

VOID
InterruptHandler
    (
        VOID
    )
{
    UINT status = *VIC_STATUS(VIC1_BASE);

    if(status & TC2IO_MASK)
    {
        InterruptpHandleClock();
    }
    else
    {
        ASSERT(FALSE);
    }
}

static
inline
VOID
InterruptpSetupTimer
    (
        VOID
    )
{
    // Set the timer load value
    // 5080 will set a 508 khz timer to fire every 10 ms
    *TIMER_LOAD(TIMER2_BASE) = 5080;

    // Enable the timer
    *TIMER_CONTROL(TIMER2_BASE) = CLKSEL_MASK | MODE_MASK | ENABLE_MASK;
}

VOID
InterruptInit
    (
        VOID
    )
{
    UINT i;

    for(i = 0; i < NumEvent; i++)
    {
        g_eventHandlers[i] = NULL;
    }

    InterruptInstallHandler();
    InterruptpSetupTimer();
}

VOID
InterruptDisableAll
    (
        VOID
    )
{
    *VIC_INTERRUPT_DISABLE(VIC1_BASE) = 0xFFFFFFFF;
}

static
inline
BOOLEAN
InterruptpIsValidEvent
    (
        IN EVENT event
    )
{
    return ClockEvent <= event && event < NumEvent;
}

static
inline
BOOLEAN
InterruptpIsEventAvailable
    (
        IN EVENT event
    )
{
    return g_eventHandlers[event] == NULL;
}

RT_STATUS
InterruptAwaitEvent
    (
        IN TASK_DESCRIPTOR* td,
        IN EVENT event
    )
{
    if(InterruptpIsValidEvent(event) &&
       InterruptpIsEventAvailable(event))
    {
        g_eventHandlers[event] = td;
        td->state = EventBlockedState;
        InterruptpEnable(event);

        return STATUS_SUCCESS;
    }
    else
    {
        ASSERT(FALSE);
        return STATUS_INVALID_PARAMETER;
    }
}
