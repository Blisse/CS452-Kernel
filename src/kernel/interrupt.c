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

#define VIC1_BASE 0x800B0000
#define STATUS_OFFSET 0
#define ENABLE_OFFSET 0x10
#define DISABLE_OFFSET 0x14

#define VIC_STATUS(vicBase) ((volatile UINT*)((vicBase) + STATUS_OFFSET))
#define VIC_INTERRUPT_ENABLE(vicBase) ((volatile UINT*)((vicBase) + ENABLE_OFFSET))
#define VIC_INTERRUPT_DISABLE(vicBase) ((volatile UINT*)((vicBase) + DISABLE_OFFSET))

#define TC2IO_MASK 0x20
#define UART1RX_MASK 0x800000
#define UART1TX_MASK 0x1000000
#define UART2RX_MASK 0x2000000
#define UART2TX_MASK 0x4000000

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
    volatile UINT* vicDisable = VIC_INTERRUPT_DISABLE(VIC1_BASE);

    switch(event)
    {
        case ClockEvent:
            *vicDisable = *vicDisable | TC2IO_MASK;
            break;

        case UartCom1ReceiveEvent:
            *vicDisable = *vicDisable | UART1RX_MASK;
            break;

        case UartCom1TransmitEvent:
            ASSERT(FALSE);
            break;

        case UartCom2ReceiveEvent:
            *vicDisable = *vicDisable | UART2RX_MASK;
            break;

        case UartCom2TransmitEvent:
            *vicDisable = *vicDisable | UART2TX_MASK;
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
    volatile UINT* vicEnable = VIC_INTERRUPT_ENABLE(VIC1_BASE);
    
    switch(event)
    {
        case ClockEvent:
            *vicEnable = *vicEnable | TC2IO_MASK;
            break;

        case UartCom1ReceiveEvent:
            *vicEnable = *vicEnable | UART1RX_MASK;
            break;

        case UartCom1TransmitEvent:
            ASSERT(FALSE);
            break;

        case UartCom2ReceiveEvent:
            *vicEnable = *vicEnable | UART2RX_MASK;
            break;

        case UartCom2TransmitEvent:
            *vicEnable = *vicEnable | UART2TX_MASK;
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
    UINT status = *VIC_STATUS(VIC1_BASE);

    if(status & TC2IO_MASK)
    {
        InterruptpHandleEvent(ClockEvent);
    }
    else if(status & UART2TX_MASK)
    {
        InterruptpHandleEvent(UartCom2TransmitEvent);
    }
    else if(status & UART2RX_MASK)
    {
        InterruptpHandleEvent(UartCom2ReceiveEvent);
    }
    else if(status & UART1RX_MASK)
    {
        InterruptpHandleEvent(UartCom1ReceiveEvent);
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
        IN UINT baudRate, 
        IN BOOLEAN needsTwoStopBits
    )
{
    volatile UINT* lcrh = UART_LCRH(uartBase);
    volatile UINT* lcrm = UART_LCRM(uartBase);
    volatile UINT* lcrl = UART_LCRL(uartBase);
    volatile UINT* ctrl = UART_CTRL(uartBase);
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
    *ctrl = *ctrl | UARTEN_MASK | MSIEN_MASK | RIEN_MASK | TIEN_MASK;
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
    InterruptpSetupUart((UINT*) UART1_BASE, 2400, TRUE);
    InterruptpSetupUart((UINT*) UART2_BASE, 115200, FALSE);
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
