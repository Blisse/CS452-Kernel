#include "display.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <rtosc/bitset.h>
#include <rtosc/buffer.h>

#include <bwio/bwio.h>

#include <user/trains.h>

#define CURSOR_MOVE "\033[%d;%dH"
#define CURSOR_CLEAR "\033[2J"
#define CURSOR_DELETE_LINE "\033[K"
#define CURSOR_HIDE "\033[?25l"

#define DISPLAY_NAME "display"

typedef enum _DISPLAY_REQUEST_TYPE
{
    DisplayCharRequest = 0,
    DisplayClockRequest,
    DisplayIdleRequest,
    DisplayLogRequest,
    DisplaySensorRequest,
    DisplayShutdownRequest,
    DisplaySwitchRequest,
} DISPLAY_REQUEST_TYPE;

typedef struct _DISPLAY_REQUEST
{
    DISPLAY_REQUEST_TYPE type;
    PVOID buffer;
    UINT bufferLength;
} DISPLAY_REQUEST;

typedef struct _DISPLAY_LOG_REQUEST
{
    CHAR message[128];
    INT messageSize;
    INT length;
} DISPLAY_LOG_REQUEST;

typedef struct _DISPLAY_SENSOR_REQUEST
{
    SENSOR_DATA data;
} DISPLAY_SENSOR_REQUEST;

typedef struct _DISPLAY_SWITCH_REQUEST
{
    INT index;
    INT number;
    SWITCH_DIRECTION direction;
} DISPLAY_SWITCH_REQUEST;

#define CURSOR_CMD_X 13
#define CURSOR_CMD_Y 2

#define CURSOR_CLOCK_X 4
#define CURSOR_CLOCK_Y 2

#define CURSOR_IDLE_X 4
#define CURSOR_IDLE_Y 4

#define CURSOR_LOG_X 25
#define CURSOR_LOG_Y 6

#define CURSOR_SWITCH_X 4
#define CURSOR_SWITCH_Y 6

#define CURSOR_SENSOR_X 16
#define CURSOR_SENSOR_Y 6

typedef struct _CURSOR_POSITION
{
    INT x;
    INT y;
} CURSOR_POSITION;

static
inline
INT
WriteCursorPosition
    (
        IN IO_DEVICE* com2Device,
        IN CURSOR_POSITION* cursor
    )
{
    return WriteFormattedString(com2Device, CURSOR_MOVE, cursor->y, cursor->x);
}

static
VOID
DisplaypCommandLine
    (
        IN IO_DEVICE* com2Device,
        IN CURSOR_POSITION* cursor,
        IN CHAR c
    )
{
    switch (c)
    {
        case '\r':
        {
            cursor->x = CURSOR_CMD_X;
            WriteCursorPosition(com2Device, cursor);
            WriteString(com2Device, CURSOR_DELETE_LINE);
            break;
        }
        case '\b':
        {
            if (cursor->x > CURSOR_CMD_X)
            {
                cursor->x--;
                WriteCursorPosition(com2Device, cursor);
                WriteString(com2Device, CURSOR_DELETE_LINE);
            }
            break;
        }
        default:
        {
            WriteCursorPosition(com2Device, cursor);
            WriteChar(com2Device, c);
            cursor->x++;
            break;
        }
    }
}

static
VOID
DisplaypClock
    (
        IN IO_DEVICE* com2Device,
        IN INT ticks
    )
{
    INT ts = (ticks / 10);
    INT s = (ts / 10);
    INT m = (s / 60);

    CURSOR_POSITION cursor = { CURSOR_CLOCK_X, CURSOR_CLOCK_Y };
    WriteCursorPosition(com2Device, &cursor);
    WriteFormattedString(com2Device, "\033[36m" "%02d:%02d:%d>" "\033[0m", m % 60, s % 60, ts % 10);
}

static
VOID
DisplaypIdlePercentage
    (
        IN IO_DEVICE* com2Device,
        IN INT idlePercentage
    )
{
    CURSOR_POSITION cursor = { CURSOR_IDLE_X, CURSOR_IDLE_Y };
    WriteCursorPosition(com2Device, &cursor);
    WriteFormattedString(com2Device, "\033[32m%02d.%02d%%" "\033[0m", idlePercentage / 100, idlePercentage % 100);
}

static
VOID
DisplaypLogRequest
    (
        IN IO_DEVICE* com2Device,
        IN RT_CIRCULAR_BUFFER* logBuffer,
        IN DISPLAY_LOG_REQUEST* logRequest
    )
{
    if (RtCircularBufferIsFull(logBuffer))
    {
        RtCircularBufferPop(logBuffer, logRequest->messageSize);
    }

    RtCircularBufferPush(logBuffer, &logRequest->message, logRequest->messageSize);

    UINT logBufferSize = RtCircularBufferSize(logBuffer) / logRequest->messageSize;

    for (UINT i = 0; i < logBufferSize; i++)
    {
        CHAR buffer[128];
        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(logBuffer, i, buffer, sizeof(buffer))));

        CURSOR_POSITION cursor = { CURSOR_LOG_X, CURSOR_LOG_Y + i };
        VERIFY(SUCCESSFUL(WriteCursorPosition(com2Device, &cursor)));
        VERIFY(SUCCESSFUL(WriteFormattedString(com2Device, CURSOR_DELETE_LINE "%s", buffer)));
    }
}

static
VOID
DisplaypSwitchRequest
    (
        IN IO_DEVICE* com2Device,
        IN DISPLAY_SWITCH_REQUEST* switchRequest
    )
{
    CURSOR_POSITION cursor = { CURSOR_SWITCH_X, CURSOR_SWITCH_Y + switchRequest->index };
    WriteCursorPosition(com2Device, &cursor);
    WriteFormattedString(com2Device, "\033[36msw\033[0m %3d \033[33m%c\033[0m", switchRequest->number, switchRequest->direction == SwitchStraight ? 'S' : 'C');
}

static
VOID
DisplaypSensorRequest
    (
        IN IO_DEVICE* com2Device,
        IN DISPLAY_SENSOR_REQUEST* sensorRequest,
        IN RT_CIRCULAR_BUFFER* sensorDataBuffer
    )
{
    SENSOR_DATA sensorData = sensorRequest->data;
    if (RtCircularBufferIsFull(sensorDataBuffer))
    {
        RtCircularBufferPop(sensorDataBuffer, sizeof(sensorData));
    }
    RtCircularBufferPush(sensorDataBuffer, &sensorData, sizeof(sensorData));

    UINT sensorDataBufferSize = RtCircularBufferSize(sensorDataBuffer) / sizeof(sensorData);
    for (UINT i = 0; i < sensorDataBufferSize; i++)
    {
        SENSOR_DATA displayData;
        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(sensorDataBuffer, i, &displayData, sizeof(displayData))));

        CURSOR_POSITION cursor = { CURSOR_SENSOR_X, CURSOR_SENSOR_Y + i };
        VERIFY(SUCCESSFUL(WriteCursorPosition(com2Device, &cursor)));
        VERIFY(SUCCESSFUL(WriteFormattedString(com2Device,
                                               CURSOR_DELETE_LINE "\033[36m%c%02d \033[33m%d\033[0m",
                                               displayData.sensor.module,
                                               displayData.sensor.number,
                                               displayData.isOn)));
    }
}

static
INT
DisplaypSendRequest
    (
        IN DISPLAY_REQUEST_TYPE type,
        IN PVOID buffer,
        IN INT bufferLength
    )
{
    DISPLAY_REQUEST request = { type, buffer, bufferLength };
    INT displayServerId = WhoIs(DISPLAY_NAME);
    ASSERT(SUCCESSFUL(displayServerId));
    return Send(displayServerId, &request, sizeof(request), NULL, 0);
}

static
VOID
DisplaypShutdownHook
    (
        VOID
    )
{
    DisplaypSendRequest(DisplayShutdownRequest, NULL, 0);
}

static
VOID
DisplaypTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(DISPLAY_NAME)));
    VERIFY(SUCCESSFUL(ShutdownRegisterHook(DisplaypShutdownHook)));

    IO_DEVICE com2Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom2, &com2Device)));

    WriteString(&com2Device, CURSOR_HIDE);
    WriteString(&com2Device, CURSOR_CLEAR);

    SENSOR_DATA underlyingSensorDataBuffer[8];
    RT_CIRCULAR_BUFFER sensorDataBuffer;
    RtCircularBufferInit(&sensorDataBuffer, underlyingSensorDataBuffer, sizeof(underlyingSensorDataBuffer));

    CHAR underlyingLogBuffer[1024];
    RT_CIRCULAR_BUFFER logBuffer;
    RtCircularBufferInit(&logBuffer, underlyingLogBuffer, sizeof(underlyingLogBuffer));

    CURSOR_POSITION cursor = { CURSOR_CMD_X, CURSOR_CMD_Y };

    BOOLEAN running = TRUE;
    while (running)
    {
        INT senderId;
        DISPLAY_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch (request.type)
        {
            case DisplayCharRequest:
            {
                CHAR c = *((CHAR*) request.buffer);
                DisplaypCommandLine(&com2Device, &cursor, c);
                break;
            }
            case DisplayClockRequest:
            {
                INT tick = *((INT*) request.buffer);
                DisplaypClock(&com2Device, tick);
                break;
            }
            case DisplayIdleRequest:
            {
                INT idle = *((INT*) request.buffer);
                DisplaypIdlePercentage(&com2Device, idle);
                break;
            }
            case DisplayLogRequest:
            {
                DISPLAY_LOG_REQUEST* logRequest = (DISPLAY_LOG_REQUEST*) request.buffer;
                DisplaypLogRequest(&com2Device, &logBuffer, logRequest);
                break;
            }
            case DisplaySwitchRequest:
            {
                DISPLAY_SWITCH_REQUEST switchRequest = *((DISPLAY_SWITCH_REQUEST*) request.buffer);
                DisplaypSwitchRequest(&com2Device, &switchRequest);
                break;
            }
            case DisplaySensorRequest:
            {
                DISPLAY_SENSOR_REQUEST sensorRequest = *((DISPLAY_SENSOR_REQUEST*) request.buffer);
                DisplaypSensorRequest(&com2Device, &sensorRequest, &sensorDataBuffer);
                break;
            }
            case DisplayShutdownRequest:
            {
                WriteString(&com2Device, CURSOR_CLEAR);
                running = FALSE;
                break;
            }
        }

        WriteCursorPosition(&com2Device, &cursor);

        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
    }
}

VOID
DisplayCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority25, DisplaypTask)));
}

VOID
ShowKeyboardChar
    (
        IN CHAR c
    )
{
    VERIFY(SUCCESSFUL(DisplaypSendRequest(DisplayCharRequest, &c, sizeof(c))));
}

VOID
ShowClockTime
    (
        IN INT clockTicks
    )
{
    VERIFY(SUCCESSFUL(DisplaypSendRequest(DisplayClockRequest, &clockTicks, sizeof(clockTicks))));
}

VOID
ShowIdleTime
    (
        IN INT idlePercentage
    )
{
    VERIFY(SUCCESSFUL(DisplaypSendRequest(DisplayIdleRequest, &idlePercentage, sizeof(idlePercentage))));
}

VOID
Log
    (
        IN STRING message,
        ...
    )
{
    DISPLAY_LOG_REQUEST logRequest;
    RtMemset(logRequest.message, sizeof(logRequest.message), 0);
    logRequest.messageSize = 128;

    VA_LIST va;
    VA_START(va, message);
    INT written = RtStrPrintFormattedVa(logRequest.message, sizeof(logRequest.message), message, va);
    VA_END(va);

    ASSERT(written < sizeof(logRequest.message));
    UNREFERENCED_PARAMETER(written);

    logRequest.length = written;

    VERIFY(SUCCESSFUL(DisplaypSendRequest(DisplayLogRequest, &logRequest, sizeof(logRequest))));
}

VOID
ShowSwitchDirection
    (
        IN INT idx,
        IN INT number,
        IN CHAR direction
    )
{
    DISPLAY_SWITCH_REQUEST switchRequest = { idx, number, direction };
    VERIFY(SUCCESSFUL(DisplaypSendRequest(DisplaySwitchRequest, &switchRequest, sizeof(switchRequest))));
}

VOID
ShowSensorStatus
    (
        IN SENSOR_DATA data
    )
{
    DISPLAY_SENSOR_REQUEST sensorRequest = { data };
    VERIFY(SUCCESSFUL(DisplaypSendRequest(DisplaySensorRequest, &sensorRequest, sizeof(sensorRequest))));
}
