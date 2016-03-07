#include "train_server.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>

#include "location_server.h"

#define TRAIN_SERVER_NAME "train"
#define NUM_TRAINS 80

#define TRAIN_COMMAND_REVERSE 0xF
#define TRAIN_COMMAND_GO 0x60
#define TRAIN_COMMAND_STOP 0x61

typedef enum _TRAIN_REQUEST_TYPE
{
    ShutdownRequest = 0,
    SetSpeedRequest,
    ReverseRequest
} TRAIN_REQUEST_TYPE;

typedef struct _TRAIN_REQUEST
{
    TRAIN_REQUEST_TYPE type;
    UCHAR train;
    UCHAR speed;
} TRAIN_REQUEST;

static
INT
TrainpSendRequest
    (
        IN TRAIN_REQUEST* request
    )
{
    INT result = WhoIs(TRAIN_SERVER_NAME);

    if(SUCCESSFUL(result))
    {
        INT trainServerId = result;

        result = Send(trainServerId, request, sizeof(*request), NULL, 0);
    }

    return result;
}

static
VOID
TrainpShutdownHook
    (
        VOID
    )
{
    TRAIN_REQUEST request = { ShutdownRequest };

    VERIFY(SUCCESSFUL(TrainpSendRequest(&request)));
}

static
INT
TrainpSendOneByteCommand
    (
        IN IO_DEVICE* device,
        IN UCHAR byte
    )
{
    return WriteChar(device, byte);
}

static
INT
TrainpSendTwoByteCommand
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
TrainpGo
    (
        IN IO_DEVICE* device
    )
{
    return TrainpSendOneByteCommand(device, TRAIN_COMMAND_GO);
}

static
INT
TrainpStop
    (
        IN IO_DEVICE* device
    )
{
    return TrainpSendOneByteCommand(device, TRAIN_COMMAND_STOP);
}

static
INT
TrainpSetSpeed
    (
        IN IO_DEVICE* device,
        IN UCHAR train,
        IN UCHAR speed
    )
{
    // Bytes must be sent in a weird order.
    // Speed first, then train.
    INT result = TrainpSendTwoByteCommand(device, speed, train);

    if(SUCCESSFUL(result))
    {
        result = LocationServerUpdateTrainSpeed(train, speed);
    }

    return result;
}

static
INT
TrainpReverse
    (
        IN IO_DEVICE* device,
        IN UCHAR train
    )
{
    INT result = TrainpSendTwoByteCommand(device, TRAIN_COMMAND_REVERSE, train);

    if(SUCCESSFUL(result))
    {
        result = LocationServerFlipTrainDirection(train);
    }

    return result;
}

static
VOID
TrainpTask
    (
        VOID
    )
{
    IO_DEVICE com1;
    UCHAR speeds[NUM_TRAINS];
    INT i;
    BOOLEAN running = TRUE;

    for(i = 0; i < NUM_TRAINS; i++)
    {
        speeds[i] = 0;
    }

    // Setup the server
    VERIFY(SUCCESSFUL(RegisterAs(TRAIN_SERVER_NAME)));
    VERIFY(SUCCESSFUL(ShutdownRegisterHook(TrainpShutdownHook)));
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1)));

    // Turn the train controller on
    VERIFY(SUCCESSFUL(TrainpGo(&com1)));

    while(running)
    {
        INT sender;
        TRAIN_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&sender, &request, sizeof(request))));

        switch(request.type)
        {
            case ShutdownRequest:
                running = FALSE;
                break;

            case SetSpeedRequest:
                VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, request.train, request.speed)));

                speeds[request.train - 1] = request.speed;
                break;

            case ReverseRequest:
            {
                UCHAR oldSpeed = speeds[request.train - 1];

                // Stop the train
                VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, request.train, 0)));

                // Wait for the train to come to a stop
                // TODO - Better implementation
                Delay(100 * (oldSpeed / 3 + 1));

                // Reverse the train
                VERIFY(SUCCESSFUL(TrainpReverse(&com1, request.train)));

                // TODO - Why do we need this, and only on track B?
                Delay(100);

                // Speed the train back up to its original speed
                VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, request.train, oldSpeed)));
                break;
            }

            default:
                ASSERT(FALSE);
                break;
        }

        VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));
    }

    // Stop any trains that are moving
    for(i = 0; i < NUM_TRAINS; i++)
    {
        if(0 != speeds[i])
        {
            VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, i + 1, 0)));
        }
    }

    // Turn the train controller off
    VERIFY(SUCCESSFUL(TrainpStop(&com1)));
}

INT
TrainSetSpeed
    (
        IN INT train,
        IN INT speed
    )
{
    if (!(0 <= train && train < NUM_TRAINS))
    {
        return -1;
    }

    if (!(0 <= speed && speed < 15))
    {
        return -1;
    }

    TRAIN_REQUEST request = { SetSpeedRequest, (UCHAR) train, (UCHAR) speed };
    return TrainpSendRequest(&request);
}

INT
TrainReverse
    (
        IN INT train
    )
{
    if (!(0 <= train && train < NUM_TRAINS))
    {
        return -1;
    }

    TRAIN_REQUEST request = { ReverseRequest, (UCHAR) train };
    return TrainpSendRequest(&request);
}

VOID
TrainServerCreate
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority20, TrainpTask)));
}
