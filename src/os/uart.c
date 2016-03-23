#include "uart.h"

#include <rtosc/assert.h>
#include <rtos.h>
#include <ts7200.h>
#include "io.h"

#define UART_COM1_READ_NAME "com1_r"
#define UART_COM1_WRITE_NAME "com1_w"
#define UART_COM2_READ_NAME "com2_r"
#define UART_COM2_WRITE_NAME "com2_w"

#define UART_DATA(uartBase) ((volatile UINT*) (uartBase + UART_DATA_OFFSET))

static
INT
UartpOpen
    (
        IN IO_CHANNEL channel,
        OUT IO_DEVICE* device
    )
{
    if (ChannelCom1 == channel)
    {
        device->readTaskId = WhoIs(UART_COM1_READ_NAME);
        ASSERT(SUCCESSFUL(device->readTaskId));

        device->writeTaskId = WhoIs(UART_COM1_WRITE_NAME);
        ASSERT(SUCCESSFUL(device->writeTaskId));

        return 0;
    }
    else if (ChannelCom2 == channel)
    {
        device->readTaskId = WhoIs(UART_COM2_READ_NAME);
        ASSERT(SUCCESSFUL(device->readTaskId));

        device->writeTaskId = WhoIs(UART_COM2_WRITE_NAME);
        ASSERT(SUCCESSFUL(device->writeTaskId));

        return 0;
    }
    else
    {
        ASSERT(FALSE);
        return -1;
    }
}

static
CHAR
UartpCom1Read()
{
    return *UART_DATA(UART1_BASE);
}

static
VOID
UartpCom1Write
    (
        CHAR c
    )
{
    *UART_DATA(UART1_BASE) = c;
}

static
CHAR
UartpCom2Read()
{
    return *UART_DATA(UART2_BASE);
}

static
VOID
UartpCom2Write
    (
        CHAR c
    )
{
    *UART_DATA(UART2_BASE) = c;
}

VOID
UartCreateTasks()
{
    // Register with the I/O framework
    VERIFY(SUCCESSFUL(IoRegisterDriver(UartDevice, UartpOpen)));

    // Create the uart I/O servers
    VERIFY(SUCCESSFUL(IoCreateReadTask(Priority29,
                                       UartCom1ReceiveEvent,
                                       UartpCom1Read,
                                       UART_COM1_READ_NAME)));
    VERIFY(SUCCESSFUL(IoCreateWriteTask(Priority29,
                                        UartCom1TransmitEvent,
                                        UartpCom1Write,
                                        UART_COM1_WRITE_NAME)));
    VERIFY(SUCCESSFUL(IoCreateReadTask(Priority11,
                                       UartCom2ReceiveEvent,
                                       UartpCom2Read,
                                       UART_COM2_READ_NAME)));
    VERIFY(SUCCESSFUL(IoCreateWriteTask(Priority11,
                                        UartCom2TransmitEvent,
                                        UartpCom2Write,
                                        UART_COM2_WRITE_NAME)));
}
