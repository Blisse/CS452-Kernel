#include "interrupt.h"

#include <rtosc/assert.h>
#include "scheduler.h"
#include <ts7200.h>

#define TIMER_CONTROL(timerBase) ((volatile UINT*)(ptr_add(timerBase, CRTL_OFFSET)))
#define TIMER_LOAD(timerBase) ((volatile UINT*)(ptr_add(timerBase, LDR_OFFSET)))

#define UART_LCRH(uartBase) ((volatile UINT*) (ptr_add(uartBase, UART_LCRH_OFFSET)))
#define UART_LCRM(uartBase) ((volatile UINT*) (ptr_add(uartBase, UART_LCRM_OFFSET)))
#define UART_LCRL(uartBase) ((volatile UINT*) (ptr_add(uartBase, UART_LCRL_OFFSET)))
#define UART_CTRL(uartBase) ((volatile UINT*) (ptr_add(uartBase, UART_CTLR_OFFSET)))
#define UART_FLAG(uartBase) ((volatile UINT*) (ptr_add(uartBase, UART_FLAG_OFFSET)))
#define UART_INTR(uartBase) ((volatile UINT*) (ptr_add(uartBase, UART_INTR_OFFSET)))

#define VIC1_BASE 0x800B0000
#define VIC2_BASE 0x800C0000
#define STATUS_OFFSET 0
#define ENABLE_OFFSET 0x10
#define DISABLE_OFFSET 0x14

#define VIC_STATUS(vicBase) ((volatile UINT*)((vicBase) + STATUS_OFFSET))
#define VIC_INTERRUPT_ENABLE(vicBase) ((volatile UINT*)((vicBase) + ENABLE_OFFSET))
#define VIC_INTERRUPT_DISABLE(vicBase) ((volatile UINT*)((vicBase) + DISABLE_OFFSET))

#define TC2IO_MASK 0x20
#define UART1_MASK 0x100000
#define UART2_MASK 0x400000

extern
VOID
InterruptInstallHandler
    (
        VOID
    );

static TASK_DESCRIPTOR* g_eventHandlers[NumEvent];

static
inline
VOID
InterruptpDisable
    (
        IN EVENT event
    )
{
    volatile UINT* vic1Disable = VIC_INTERRUPT_DISABLE(VIC1_BASE);
    volatile UINT* uart1Ctrl = UART_CTRL((UINT*) UART1_BASE);
    volatile UINT* uart2Ctrl = UART_CTRL((UINT*) UART2_BASE);

    switch(event)
    {
        case ClockEvent:
            *vic1Disable = *vic1Disable | TC2IO_MASK;
            break;

        case UartCom1ReceiveEvent:
            *uart1Ctrl = *uart1Ctrl & ~RIEN_MASK;
            break;

        case UartCom1TransmitEvent:
            *uart1Ctrl = *uart1Ctrl & ~TIEN_MASK;
            break;

        case UartCom2ReceiveEvent:
            *uart2Ctrl = *uart2Ctrl & ~RIEN_MASK;
            break;

        case UartCom2TransmitEvent:
            *uart2Ctrl = *uart2Ctrl & ~TIEN_MASK;
            break;

        default:
            ASSERT(FALSE);
            break;
    }
} 

static
inline
VOID
InterruptpEnable
    (
        IN EVENT event
    )
{
    volatile UINT* vic1Enable = VIC_INTERRUPT_ENABLE(VIC1_BASE);
    volatile UINT* uart1Ctrl = UART_CTRL((UINT*) UART1_BASE);
    volatile UINT* uart2Ctrl = UART_CTRL((UINT*) UART2_BASE);

    switch(event)
    {
        case ClockEvent:
            *vic1Enable = *vic1Enable | TC2IO_MASK;
            break;

        case UartCom1ReceiveEvent:
            *uart1Ctrl = *uart1Ctrl | RIEN_MASK;
            break;

        case UartCom1TransmitEvent:
            *uart1Ctrl = *uart1Ctrl | TIEN_MASK;
            break;

        case UartCom2ReceiveEvent:
            *uart2Ctrl = *uart2Ctrl | RIEN_MASK;
            break;

        case UartCom2TransmitEvent:
            *uart2Ctrl = *uart2Ctrl | TIEN_MASK;
            break;

        default:
            ASSERT(FALSE);
            break;
    }
}

static
inline
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
InterruptpHandleEvent
    (
        IN EVENT event
    )
{
    // Handle the interrupt
    InterruptpSignalEvent(event);

    // Disable the interrupt
    InterruptpDisable(event);
}

VOID
InterruptHandler
    (
        VOID
    )
{
    UINT vic1Status = *VIC_STATUS(VIC1_BASE);
    UINT vic2Status = *VIC_STATUS(VIC2_BASE);

    if(vic1Status & TC2IO_MASK)
    {
        InterruptpHandleEvent(ClockEvent);
    }
    else if(vic2Status & UART2_MASK)
    {
        UINT uart2Status = *UART_INTR((UINT*) UART2_BASE);

        if(uart2Status & TIS_MASK)
        {
            InterruptpHandleEvent(UartCom2TransmitEvent);
        }
        else if(uart2Status & RIS_MASK)
        {
            InterruptpHandleEvent(UartCom2ReceiveEvent);
        }
        else
        {
            ASSERT(FALSE);
        }
    }
    else if(vic2Status & UART1_MASK)
    {
        static BOOLEAN cts = TRUE;
        UINT uart1Status = *UART_INTR((UINT*) UART1_BASE);

        if(uart1Status & TIS_MASK && cts)
        {
            InterruptpHandleEvent(UartCom1TransmitEvent);
        }
        else if(uart1Status & MIS_MASK)
        {
            // Acknowledge the interrupt
            *UART_INTR((UINT*) UART1_BASE) = TRUE;

            // Get the data from the interrupt
            cts = *UART_FLAG((UINT*) UART1_BASE) & CTS_MASK;
        }
        else if(uart1Status & RIS_MASK)
        {
            InterruptpHandleEvent(UartCom1ReceiveEvent);
        }
        else
        {
            ASSERT(FALSE);
        }
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
        IN UINT* timerBase
    )
{
    // Set the timer load value
    // 5080 will set a 508 khz timer to fire every 10 ms
    *TIMER_LOAD(timerBase) = 5080;

    // Enable the timer
    *TIMER_CONTROL(timerBase) = CLKSEL_MASK | MODE_MASK | ENABLE_MASK;
}

static
inline
VOID
InterruptpSetupUart
    (
        IN UINT* uartBase, 
        IN UINT vicMask,
        IN UINT baudRate, 
        IN BOOLEAN needsTwoStopBits
    )
{
    volatile UINT* lcrh = UART_LCRH(uartBase);
    volatile UINT* lcrm = UART_LCRM(uartBase);
    volatile UINT* lcrl = UART_LCRL(uartBase);
    volatile UINT* ctrl = UART_CTRL(uartBase);
    volatile UINT* vic2Enable = VIC_INTERRUPT_ENABLE(VIC2_BASE);
    USHORT baudRateDivisor = (7372800 / (16 * baudRate)) - 1;

    // Disable FIFO
    *lcrh = *lcrh & ~FEN_MASK;

    // Disable parity bits
    *lcrh = *lcrh & ~PEN_MASK;

    // Setup stop bits
    if(needsTwoStopBits)
    {
        *lcrh = *lcrh | STP2_MASK;
    }
    else
    {
        *lcrh = *lcrh & ~STP2_MASK;
    }

    // Setup the baud rate
    *lcrm = baudRateDivisor >> 8;
    *lcrl = baudRateDivisor & 0xFF;

    // Enable the uart
    *ctrl = *ctrl | UARTEN_MASK | MSIEN_MASK;
    *vic2Enable = *vic2Enable | vicMask;
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
    InterruptpSetupTimer((UINT*) TIMER2_BASE);
    InterruptpSetupUart((UINT*) UART1_BASE, UART1_MASK, 2400, TRUE);
    InterruptpSetupUart((UINT*) UART2_BASE, UART2_MASK, 115200, FALSE);
}

VOID
InterruptDisableAll
    (
        VOID
    )
{
    *VIC_INTERRUPT_DISABLE(VIC1_BASE) = 0xFFFFFFFF;
    *VIC_INTERRUPT_DISABLE(VIC2_BASE) = 0xFFFFFFFF;
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
