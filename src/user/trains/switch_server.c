#include "switch_server.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>
#include <user/io.h>

#define SWITCH_SERVER_NAME "switch"

#define NUM_SWITCHES 22
#define SWITCH_COMMAND_DISABLE_SOLENOID 0x20
#define SWITCH_COMMAND_DIRECTION_STRAIGHT 0x21
#define SWITCH_COMMAND_DIRECTION_CURVED 0x22

typedef enum _SWITCH_REQUEST_TYPE
{
    SetDirectionRequest = 0,
    GetDirectionRequest
} SWITCH_REQUEST_TYPE;

typedef struct _SWITCH_REQUEST
{
    SWITCH_REQUEST_TYPE type;
    UCHAR sw;
    SWITCH_DIRECTION direction;
} SWITCH_REQUEST;

// Really hacky conversion of a switch to an index in a buffer
static
UINT
SwitchpToIndex
    (
        IN UCHAR sw
    )
{
    if (1 <= sw && sw <= 18)
    {
        return sw - 1;
    }
    else
    {
        // 153 -> 156
        return sw - 135;
    }
}

static
UCHAR
SwitchpFromIndex
    (
        IN UINT index
    )
{
    if (0 <= index && index <= 17)
    {
        return index + 1;
    }
    else
    {
        return index + 135;
    }
}

static
INT
SwitchpSendOneByteCommand
    (
        IN IO_DEVICE* device,
        IN UCHAR byte
    )
{
    return WriteChar(device, byte);
}

static
INT
SwitchpSendTwoByteCommand
    (
        IN IO_DEVICE* device,
        IN UCHAR byte1,
        IN UCHAR byte2
    )
{
    UCHAR buffer[2] = { byte1, byte2 };

    return Write(device, buffer, sizeof(buffer));
}

static
INT
SwitchpDirection
    (
        IN IO_DEVICE* device,
        IN UCHAR sw,
        IN SWITCH_DIRECTION direction
    )
{
    if (SwitchCurved == direction)
    {
        return SwitchpSendTwoByteCommand(device, SWITCH_COMMAND_DIRECTION_CURVED, sw);
    }
    else
    {
        return SwitchpSendTwoByteCommand(device, SWITCH_COMMAND_DIRECTION_STRAIGHT, sw);
    }
}

static
INT
SwitchpDisableSolenoid
    (
        IN IO_DEVICE* device
    )
{
    return SwitchpSendOneByteCommand(device, SWITCH_COMMAND_DISABLE_SOLENOID);
}

static
VOID
SwitchpTask()
{
    SWITCH_DIRECTION directions[NUM_SWITCHES];

    VERIFY(SUCCESSFUL(RegisterAs(SWITCH_SERVER_NAME)));

    IO_DEVICE com1;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1)));

    for (UINT i = 0; i < NUM_SWITCHES; i++)
    {
        UCHAR sw = SwitchpFromIndex(i);

        VERIFY(SUCCESSFUL(SwitchpDirection(&com1, sw, SwitchCurved)));

        directions[i] = SwitchCurved;

        ShowSwitchDirection(i, sw, SwitchCurved);
    }

    VERIFY(SUCCESSFUL(SwitchpDisableSolenoid(&com1)));

    while (1)
    {
        INT senderId;
        SWITCH_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch(request.type)
        {
            case SetDirectionRequest:
            {
                VERIFY(SUCCESSFUL(SwitchpDirection(&com1, request.sw, request.direction)));
                VERIFY(SUCCESSFUL(SwitchpDisableSolenoid(&com1)));

                UINT switchIndex = SwitchpToIndex(request.sw);
                directions[switchIndex] = request.direction;

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                ShowSwitchDirection(switchIndex, request.sw, request.direction);

                break;
            }

            case GetDirectionRequest:
            {
                UINT switchIndex = SwitchpToIndex(request.sw);
                SWITCH_DIRECTION direction = directions[switchIndex];
                VERIFY(SUCCESSFUL(Reply(senderId, &direction, sizeof(direction))));
                break;
            }
        }
    }
}

VOID
SwitchServerCreate()
{
    VERIFY(SUCCESSFUL(Create(Priority19, SwitchpTask)));
}

INT
SwitchSetDirection
    (
        IN INT sw,
        IN SWITCH_DIRECTION direction
    )
{
    INT result;
    UINT index = SwitchpToIndex(sw);

    if (index < NUM_SWITCHES)
    {
        result = WhoIs(SWITCH_SERVER_NAME);

        if (SUCCESSFUL(result))
        {
            INT switchServerId = result;
            SWITCH_REQUEST request = { SetDirectionRequest, (UCHAR) sw, direction };

            result = Send(switchServerId, &request, sizeof(request), NULL, 0);
        }
    }
    else
    {
        result = -1;
    }

    return result;
}

INT
SwitchGetDirection
    (
        IN INT sw,
        OUT SWITCH_DIRECTION* direction
    )
{
    INT result;
    UINT index = SwitchpToIndex(sw);

    if (index < NUM_SWITCHES)
    {
        result = WhoIs(SWITCH_SERVER_NAME);

        if (SUCCESSFUL(result))
        {
            INT switchServerId = result;
            SWITCH_REQUEST request = { GetDirectionRequest, (UCHAR) sw };

            result = Send(switchServerId, &request, sizeof(request), direction, sizeof(*direction));
        }
    }
    else
    {
        result = -1;
    }

    return result;
}
