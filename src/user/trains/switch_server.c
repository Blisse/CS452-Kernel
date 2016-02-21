#include "switch_server.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>

#include <user/trains.h>

#include "display.h"

#define SWITCH_SERVER_NAME "switch"

#define NUM_SWITCHES 22
#define SWITCH_COMMAND_DISABLE_SOLENOID 0x20
#define SWITCH_COMMAND_DIRECTION_STRAIGHT 0x21
#define SWITCH_COMMAND_DIRECTION_CURVED 0x22

typedef enum _SWITCH_REQUEST_TYPE
{
    SetDirectionRequest = 0
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
    if(1 <= sw && sw <= 18)
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
    if(0 <= index && index <= 17)
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
    // Need to send bytes in a weird order.
    // Send the direction first, then the switch
    if(SwitchCurved == direction)
    {
        return SwitchpSendTwoByteCommand(device,
                                         SWITCH_COMMAND_DIRECTION_CURVED,
                                         sw);
    }
    else
    {
        return SwitchpSendTwoByteCommand(device,
                                         SWITCH_COMMAND_DIRECTION_STRAIGHT,
                                         sw);
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
SwitchpTask
    (
        VOID
    )
{
    INT i;
    SWITCH_DIRECTION directions[NUM_SWITCHES];
    IO_DEVICE com1;

    // Setup the server
    VERIFY(SUCCESSFUL(RegisterAs(SWITCH_SERVER_NAME)));
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1)));

    // Set the switches to a known state
    for(i = 0; i < NUM_SWITCHES; i++)
    {
        UCHAR sw = SwitchpFromIndex(i);

        VERIFY(SUCCESSFUL(SwitchpDirection(&com1, sw, SwitchCurved)));

        directions[i] = SwitchCurved;

        ShowSwitchDirection(i, sw, 'C');
    }

    // Remember to turn off the solenoid!
    VERIFY(SUCCESSFUL(SwitchpDisableSolenoid(&com1)));

    // Run the server
    while(1)
    {
        INT sender;
        SWITCH_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&sender, &request, sizeof(request))));

        switch(request.type)
        {
            case SetDirectionRequest:
                // Turn the switch direction
                VERIFY(SUCCESSFUL(SwitchpDirection(&com1,
                                                   request.sw,
                                                   request.direction)));

                // Turn off the solenoid
                VERIFY(SUCCESSFUL(SwitchpDisableSolenoid(&com1)));

                // Update the switch direction
                INT switchIndex = SwitchpToIndex(request.sw);
                directions[switchIndex] = request.direction;

                // Reply to the sender
                VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));

                ShowSwitchDirection(switchIndex, request.sw, request.direction == SwitchCurved ? 'C' : 'S');

                break;

            default:
                ASSERT(FALSE);
                break;
        }
    }
}

static
INT
SwitchpSendRequest
    (
        IN SWITCH_REQUEST* request
    )
{
    INT result = WhoIs(SWITCH_SERVER_NAME);

    if(SUCCESSFUL(result))
    {
        INT switchServerId = result;

        result = Send(switchServerId,
                      request,
                      sizeof(*request),
                      NULL,
                      0);
    }

    return result;
}

INT
SwitchSetDirection
    (
        IN INT sw,
        IN SWITCH_DIRECTION direction
    )
{
    INT index = SwitchpToIndex(sw);
    if (!(0 <= index && index < NUM_SWITCHES))
    {
        return -1;
    }

    SWITCH_REQUEST request = { SetDirectionRequest, (UCHAR) sw, direction };
    return SwitchpSendRequest(&request);
}

VOID
SwitchServerCreate
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority19, SwitchpTask)));
}
