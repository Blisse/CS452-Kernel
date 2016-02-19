#include "display.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>

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

typedef struct _DISPLAY_SWITCH_REQUEST
{
    INT index;
    INT number;
    CHAR direction;
} DISPLAY_SWITCH_REQUEST;

VOID
DisplaypTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(DISPLAY_NAME)));

    IO_DEVICE com2Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom2, &com2Device)));

    while (1)
    {
        INT senderId;
        DISPLAY_REQUEST request;
        Receive(&senderId, &request, sizeof(request));

        switch (request.type)
        {
            case DisplayCharRequest:
                break;
            case DisplayClockRequest:
                break;
            case DisplayIdleRequest:
                break;
            case DisplaySwitchRequest:
                break;
            case DisplaySensorRequest:
                break;
        }
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
