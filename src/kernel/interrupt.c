#include "interrupt.h"

#include <bwio/bwio.h>
#include <rtosc/assert.h>
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

static UINT g_numInterrupts;

static
inline
VOID
InterruptHandleClock
    (
        VOID
    )
{
    // Acknowledge the interrupt
    *TIMER_CLEAR(TIMER2_BASE) = TRUE;

    // Handle the interrupt
    g_numInterrupts++;
    bwprintf(BWCOM2, "%d\r\n", g_numInterrupts);

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
        InterruptHandleClock();
    }
    else
    {
        ASSERT(FALSE, "Unknown interrupt");
    }
}

static
inline
VOID
InterruptSetupTimer
    (
        VOID
    )
{
    // Set the timer load value
    // 5080 will set a 508 khz timer to fire every 10 ms
    *TIMER_LOAD(TIMER2_BASE) = 5080;

    // Enable the timer
    *TIMER_CONTROL(TIMER2_BASE) = CLKSEL_MASK | MODE_MASK | ENABLE_MASK;

    // Enable the timer interrupt
    volatile UINT* vicEnable = VIC_INTERRUPT_ENABLE(VIC1_BASE);
    *vicEnable = *vicEnable | TC2IO_MASK;
}

VOID
InterruptInit
    (
        VOID
    )
{
    g_numInterrupts = 0;

    // Install our interrupt handler
    InterruptInstallHandler();
}

VOID
InterruptEnable
    (
        VOID
    )
{
    // Setup interrupt handlers
    InterruptSetupTimer();
}

VOID
InterruptDisable
    (
        VOID
    )
{
    *VIC_INTERRUPT_DISABLE(VIC1_BASE) = 0xFFFFFFFF;
}
