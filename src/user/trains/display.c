#include "display.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/string.h>

#include <bwio/bwio.h>

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
    DisplaySensorRequest,
    DisplaySwitchRequest,
} DISPLAY_REQUEST_TYPE;

typedef struct _DISPLAY_REQUEST
{
    DISPLAY_REQUEST_TYPE type;
    PVOID buffer;
    UINT bufferLength;
} DISPLAY_REQUEST;

typedef struct _DISPLAY_SENSOR_REQUEST
{
    INT index;
    CHAR sensorState;
} DISPLAY_SENSOR_REQUEST;

typedef struct _DISPLAY_SWITCH_REQUEST
{
    INT index;
    INT number;
    CHAR direction;
} DISPLAY_SWITCH_REQUEST;

#define CURSOR_CMD_X 13
#define CURSOR_CMD_Y 2

#define CURSOR_CLOCK_X 4
#define CURSOR_CLOCK_Y 2

#define CURSOR_IDLE_X 4
#define CURSOR_IDLE_Y 4

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
VOID
DisplaypMoveToCursor
    (
        IN IO_DEVICE* com2Device,
        IN CURSOR_POSITION* cursor
    )
{
    CHAR buffer[10];
    RtStrPrintFormatted(buffer, sizeof(buffer), CURSOR_MOVE, cursor->y, cursor->x);
    WriteString(com2Device, buffer);
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
            DisplaypMoveToCursor(com2Device, cursor);
            WriteString(com2Device, CURSOR_DELETE_LINE);
            break;
        }
        case '\b':
        {
            if (cursor->x > CURSOR_CMD_X)
            {
                cursor->x--;
                DisplaypMoveToCursor(com2Device, cursor);
                WriteString(com2Device, CURSOR_DELETE_LINE);
            }
            break;
        }
        default:
        {
            DisplaypMoveToCursor(com2Device, cursor);
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
    DisplaypMoveToCursor(com2Device, &cursor);
    WriteFormattedString(com2Device, "\033[31m" "%02d:%02d:%d>" "\033[0m", m % 60, s % 60, ts % 10);
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
    DisplaypMoveToCursor(com2Device, &cursor);
    WriteFormattedString(com2Device, "\033[33m" "%02d.%02d%%" "\033[0m", idlePercentage / 100, idlePercentage % 100);
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
    DisplaypMoveToCursor(com2Device, &cursor);
    WriteFormattedString(com2Device, "sw%03d %c", switchRequest->number, switchRequest->direction);
}

static
VOID
DisplaypSensorRequest
    (
        IN IO_DEVICE* com2Device,
        IN DISPLAY_SENSOR_REQUEST* sensorRequest
    )
{
    CURSOR_POSITION cursor = { CURSOR_SENSOR_X, CURSOR_SENSOR_Y + sensorRequest->index };
    DisplaypMoveToCursor(com2Device, &cursor);
    WriteFormattedString(com2Device, "s%02d", sensorRequest->sensorState);
}

static
VOID
DisplaypShutdownHook
    (
        VOID
    )
{
    IO_DEVICE com2Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom2, &com2Device)));

    WriteString(&com2Device, CURSOR_CLEAR);
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

    CURSOR_POSITION cursor = { CURSOR_CMD_X, CURSOR_CMD_Y };

    while (1)
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
            case DisplaySwitchRequest:
            {
                DISPLAY_SWITCH_REQUEST switchRequest = *((DISPLAY_SWITCH_REQUEST*) request.buffer);
                DisplaypSwitchRequest(&com2Device, &switchRequest);
                break;
            }
            case DisplaySensorRequest:
            {
                DISPLAY_SENSOR_REQUEST sensorRequest = *((DISPLAY_SENSOR_REQUEST*) request.buffer);
                DisplaypSensorRequest(&com2Device, &sensorRequest);
                break;
            }
        }

        DisplaypMoveToCursor(&com2Device, &cursor);

        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
    }
}

VOID
DisplayCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(HighestUserPriority, DisplaypTask)));
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

    return Send(displayServerId,
                &request,
                sizeof(request),
                NULL,
                0);
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
ShowSensorState
    (
        IN INT idx,
        IN CHAR sensorState
    )
{
    DISPLAY_SENSOR_REQUEST sensorRequest = { idx, sensorState };
    VERIFY(SUCCESSFUL(DisplaypSendRequest(DisplaySensorRequest, &sensorRequest, sizeof(sensorRequest))));
}
