#include "uart.h"
#include "io.h"
#include "ts7200.h"

#define UART_CLK 7372800
#define COM1_BAUD_RATE 2400
#define COM2_BAUD_RATE 115200

#define COM1_READ_BUFFER_SIZE 128
#define COM1_WRITE_BUFFER_SIZE 512

#define COM2_READ_BUFFER_SIZE 256
#define COM2_WRITE_BUFFER_SIZE 1024

#define UartpData(uart) ((UINT*) ptr_add((uart)->baseAddr, UART_DATA_OFFSET))
#define UartpLineControlHigh(uart) ((UINT*) ptr_add((uart)->baseAddr, UART_LCRH_OFFSET))
#define UartpLineControlMid(uart) ((UINT*) ptr_add((uart)->baseAddr, UART_LCRM_OFFSET))
#define UartpLineControlLow(uart) ((UINT*) ptr_add((uart)->baseAddr, UART_LCRL_OFFSET))
#define UartpFlag(uart) ((UINT*) ptr_add((uart)->baseAddr, UART_FLAG_OFFSET))

#define UartCTS(uart, flag) (!uart->checkCTS || (uart->checkCTS && (flag & CTS_MASK)))

typedef struct _UART_DEVICE
{
    IO_DEVICE ioDevice;
    UINT* baseAddr;
    BOOLEAN checkCTS;
} UART_DEVICE;

#define IO_DEVICE_TO_UART_DEVICE(device) container_of(device, UART_DEVICE, ioDevice)

static CHAR Com1ReadBuffer[COM1_READ_BUFFER_SIZE];
static CHAR Com1WriteBuffer[COM1_WRITE_BUFFER_SIZE];
static UART_DEVICE Com1;

static CHAR Com2ReadBuffer[COM2_READ_BUFFER_SIZE];
static CHAR Com2WriteBuffer[COM2_WRITE_BUFFER_SIZE];
static UART_DEVICE Com2;

static
VOID
UartpSetFIFO
    (
        IN UART_DEVICE* uart,
        IN BOOLEAN isEnabled
    )
{
    UINT* lineControlHigh = UartpLineControlHigh(uart);

    if(isEnabled)
    {
        *lineControlHigh = *lineControlHigh | FEN_MASK;
    }
    else
    {
        *lineControlHigh = *lineControlHigh & ~FEN_MASK;
    }
}

static
VOID
UartpSetParityEnable
    (
        IN UART_DEVICE* uart,
        IN BOOLEAN parityEnable
    )
{
    UINT* lineControlHigh = UartpLineControlHigh(uart);

    if(parityEnable)
    {
        *lineControlHigh = *lineControlHigh | PEN_MASK;
    }
    else
    {
        *lineControlHigh = *lineControlHigh & ~PEN_MASK;
    }
}

static
VOID
UartpSetTwoStopBits
    (
        IN UART_DEVICE* uart,
        IN BOOLEAN twoStopBits
    )
{
    UINT* lineControlHigh = UartpLineControlHigh(uart);

    if(twoStopBits)
    {
        *lineControlHigh = *lineControlHigh | STP2_MASK;
    }
    else
    {
        *lineControlHigh = *lineControlHigh & ~STP2_MASK;
    }
}

static
VOID
UartpSetBaudRate
    (
        IN UART_DEVICE* uart,
        IN UINT baudRate
    )

{
    USHORT divisor = (UART_CLK / (16 * baudRate)) - 1;
    *UartpLineControlMid(uart) = divisor >> 8;
    *UartpLineControlLow(uart) = divisor & 0xFF;
}

static
RT_STATUS
UartpWrite
    (
        IN IO_DEVICE* device,
        IN CHAR byte
    )
{
    UART_DEVICE* uart = IO_DEVICE_TO_UART_DEVICE(device);
    UINT* data = UartpData(uart);
    UINT flag = *UartpFlag(uart);
    RT_STATUS status;

    if(!(flag & TXFF_MASK) &&
       UartCTS(uart, flag))
    {
        *data = byte;
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_DEVICE_NOT_READY;
    }

    return status;
}

static
VOID
UartpDeviceInit
    (
        IN UART_DEVICE* uart,
        IN HANDLE handle,
        IN PVOID readBuffer,
        IN UINT readBufferSize,
        IN PVOID writeBuffer,
        IN UINT writeBufferSize,
        IN UINT* baseAddr,
        IN UINT baudRate,
        IN BOOLEAN checkCTS,
        IN BOOLEAN needsTwoStopBits
    )
{
    IoDeviceInit(&uart->ioDevice,
                 handle,
                 UartpWrite,
                 readBuffer,
                 readBufferSize,
                 writeBuffer,
                 writeBufferSize);

    uart->baseAddr = baseAddr;
    uart->checkCTS = checkCTS;
    UartpSetFIFO(uart, FALSE);
    UartpSetParityEnable(uart, FALSE);
    UartpSetBaudRate(uart, baudRate);
    UartpSetTwoStopBits(uart, needsTwoStopBits);
}

RT_STATUS
UartDriverInit
    (
        VOID
    )
{
    UartpDeviceInit(&Com1,
                    COM1_HANDLE,
                    Com1ReadBuffer,
                    sizeof(Com1ReadBuffer),
                    Com1WriteBuffer,
                    sizeof(Com1WriteBuffer),
                    (UINT*) UART1_BASE,
                    COM1_BAUD_RATE,
                    TRUE,
                    TRUE);

    UartpDeviceInit(&Com2,
                    COM2_HANDLE,
                    Com2ReadBuffer,
                    sizeof(Com2ReadBuffer),
                    Com2WriteBuffer,
                    sizeof(Com2WriteBuffer),
                    (UINT*) UART2_BASE,
                    COM2_BAUD_RATE,
                    FALSE,
                    FALSE);

    return STATUS_SUCCESS;
}

// Temporary for A0
// This will be changed to an interrupt handler in A1
static
VOID
UartpCheckDevice
    (
        IN UART_DEVICE* uart
    )
{
    UINT flag = *UartpFlag(uart);

    if(!(flag & TXFF_MASK))
    {
        if(IoDeviceIsWriting(&uart->ioDevice))
        {
            IoWriteFinished(&uart->ioDevice);
        }
        else
        {
            // Temporary for A0
            IoFlush(&uart->ioDevice);
        }
    }

    if(flag & RXFF_MASK)
    {
        CHAR buffer = *UartpData(uart);
        IoReceiveData(&uart->ioDevice, &buffer, sizeof(buffer));
    }
}

// Temporary for A0
// This will be removed in A1
RT_STATUS
UartPollingUpdate
    (
        VOID
    )
{
    UartpCheckDevice(&Com1);
    UartpCheckDevice(&Com2);

    return STATUS_SUCCESS;
}
