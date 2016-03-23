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
#define UART_CTS(uartBase) (*UART_FLAG(uartBase) & CTS_MASK)

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

#define UART_CLK 7372800

extern
VOID
InterruptInstallHandler();

static TASK_DESCRIPTOR* g_eventHandlers[NumEvent];
static volatile BOOLEAN g_clearToSend;
static volatile BOOLEAN g_transmitReady;

static
inline
VOID
InterruptpDisable
    (
        IN EVENT event
    )
{
    switch(event)
    {
        case ClockEvent:
            *VIC_INTERRUPT_DISABLE(VIC1_BASE) |= TC2IO_MASK;
            break;

        case UartCom1ReceiveEvent:
            *UART_CTRL((UINT*) UART1_BASE) &= ~RIEN_MASK;
            break;

        case UartCom1TransmitEvent:
            *UART_CTRL((UINT*) UART1_BASE) &= ~TIEN_MASK;
            break;

        case UartCom2ReceiveEvent:
            *UART_CTRL((UINT*) UART2_BASE) &= ~RIEN_MASK;
            break;

        case UartCom2TransmitEvent:
            *UART_CTRL((UINT*) UART2_BASE) &= ~TIEN_MASK;
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
    switch(event)
    {
        case ClockEvent:
            *VIC_INTERRUPT_ENABLE(VIC1_BASE) |= TC2IO_MASK;
            break;

        case UartCom1ReceiveEvent:
            *UART_CTRL((UINT*) UART1_BASE) |= RIEN_MASK;
            break;

        case UartCom1TransmitEvent:
            *UART_CTRL((UINT*) UART1_BASE) |= TIEN_MASK;
            break;

        case UartCom2ReceiveEvent:
            *UART_CTRL((UINT*) UART2_BASE) |= RIEN_MASK;
            break;

        case UartCom2TransmitEvent:
            *UART_CTRL((UINT*) UART2_BASE) |= TIEN_MASK;
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
InterruptHandler()
{
    if (*VIC_STATUS(VIC1_BASE) & TC2IO_MASK)
    {
        InterruptpHandleEvent(ClockEvent);
    }
    else if (*VIC_STATUS(VIC2_BASE) & UART2_MASK)
    {
        UINT uart2Status = *UART_INTR((UINT*) UART2_BASE);

        if (uart2Status & TIS_MASK)
        {
            InterruptpHandleEvent(UartCom2TransmitEvent);
        }

        if (uart2Status & RIS_MASK)
        {
            InterruptpHandleEvent(UartCom2ReceiveEvent);
        }
    }
    else if (*VIC_STATUS(VIC2_BASE) & UART1_MASK)
    {
        UINT uart1Status = *UART_INTR((UINT*) UART1_BASE);

        if (uart1Status & TIS_MASK)
        {
            g_transmitReady = TRUE;
            InterruptpDisable(UartCom1TransmitEvent);
        }

        if (uart1Status & MIS_MASK)
        {
            // Acknowledge the interrupt
            *UART_INTR((UINT*) UART1_BASE) = TRUE;

            // Get the data from the interrupt
            g_clearToSend = UART_CTS((UINT*) UART1_BASE);
        }

        if (uart1Status & RIS_MASK)
        {
            InterruptpHandleEvent(UartCom1ReceiveEvent);
        }

        if (g_clearToSend && g_transmitReady)
        {
            InterruptpSignalEvent(UartCom1TransmitEvent);
            g_transmitReady = FALSE;
            g_clearToSend = FALSE;
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
    // Setup the baud rate
    USHORT baudRateDivisor = (UART_CLK / (16 * baudRate)) - 1;
    *UART_LCRM(uartBase) = baudRateDivisor >> 8;
    *UART_LCRL(uartBase) = baudRateDivisor & 0xFF;

    // Setup the uart
    // Disable FIFO and parity bits
    *UART_LCRH(uartBase) &= ~(FEN_MASK | PEN_MASK);

    if (needsTwoStopBits)
    {
        *UART_LCRH(uartBase) |= STP2_MASK;
    }
    else
    {
        *UART_LCRH(uartBase) &= ~STP2_MASK;
    }

    // Enable interrupts
    *UART_CTRL(uartBase) |= UARTEN_MASK | MSIEN_MASK;
    *VIC_INTERRUPT_ENABLE(VIC2_BASE) |= vicMask;
}

VOID
InterruptInit()
{
    UINT i;

    for (i = 0; i < NumEvent; i++)
    {
        g_eventHandlers[i] = NULL;
    }

    g_clearToSend = UART_CTS((UINT*) UART1_BASE);
    g_transmitReady = FALSE;

    InterruptInstallHandler();
    InterruptpSetupTimer((UINT*) TIMER2_BASE);
    InterruptpSetupUart((UINT*) UART1_BASE, UART1_MASK, 2400, TRUE);
    InterruptpSetupUart((UINT*) UART2_BASE, UART2_MASK, 115200, FALSE);
}

VOID
InterruptDisableAll()
{
    // Disable the interrupt vectors
    *VIC_INTERRUPT_DISABLE(VIC1_BASE) = 0xFFFFFFFF;
    *VIC_INTERRUPT_DISABLE(VIC2_BASE) = 0xFFFFFFFF;

    // Disable any pending uart interrupts
    InterruptpDisable(UartCom1ReceiveEvent);
    InterruptpDisable(UartCom1TransmitEvent);
    InterruptpDisable(UartCom2ReceiveEvent);
    InterruptpDisable(UartCom2TransmitEvent);
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
    if (InterruptpIsValidEvent(event) &&
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
