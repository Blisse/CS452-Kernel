#include "train_server.h"

#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <rtkernel.h>
#include <rtos.h>
#include <user/io.h>
#include <user/trains.h>

#include "location_server.h"

#define TRAIN_SERVER_NAME "train"
#define NUM_TRAINS 80

#define TRAIN_COMMAND_REVERSE 0xF
#define TRAIN_COMMAND_GO 0x60
#define TRAIN_COMMAND_STOP 0x61

typedef enum _TRAIN_REQUEST_TYPE {
    ShutdownRequest = 0,
    SendDataRequest,
    SetSpeedRequest,
    ReverseRequest
} TRAIN_REQUEST_TYPE;

typedef struct _TRAIN_REQUEST {
    TRAIN_REQUEST_TYPE type;
    UCHAR train;
    UCHAR speed;
} TRAIN_REQUEST;

static
INT
TrainpSendRequest(
        IN TRAIN_REQUEST* request
    )
{
    INT trainServerId = WhoIs(TRAIN_SERVER_NAME);

    return Send(trainServerId, request, sizeof(*request), NULL, 0);
}

static
VOID
TrainpShutdownHook()
{
    TRAIN_REQUEST request = { ShutdownRequest };

    VERIFY(SUCCESSFUL(TrainpSendRequest(&request)));
}

static
INT
TrainpSendOneByteCommand(
        IN IO_DEVICE* device,
        IN UCHAR byte
    )
{
    return WriteChar(device, byte);
}

static
INT
TrainpSendTwoByteCommand(
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
TrainpGo(
        IN IO_DEVICE* device
    )
{
    return TrainpSendOneByteCommand(device, TRAIN_COMMAND_GO);
}

static
INT
TrainpStop(
        IN IO_DEVICE* device
    )
{
    return TrainpSendOneByteCommand(device, TRAIN_COMMAND_STOP);
}

static
INT
TrainpSetSpeed(
        IN IO_DEVICE* device,
        IN UCHAR train,
        IN UCHAR speed
    )
{
    return TrainpSendTwoByteCommand(device, speed, train);
}

static
INT
TrainpReverse(
        IN IO_DEVICE* device,
        IN UCHAR train
    )
{
    return TrainpSendTwoByteCommand(device, TRAIN_COMMAND_REVERSE, train);
}

// 64 all off
// 65 #1
// 66 #2
// 67 #1,#2
// 68 #3
// 69 #3,#1
// 70 #3,#2
// 71 #3,#2,#1
// 72 #4
// 73 #4,#1
// 74 #4,#2
// 75 #4,#2,#1
// 76 #4,#3
// 77 #4,#3,#1
// 78 #4,#3,#2
// 79 #4,#3,#2,#1

static
VOID
TrainpTask()
{
    VERIFY(SUCCESSFUL(RegisterAs(TRAIN_SERVER_NAME)));
    VERIFY(SUCCESSFUL(ShutdownRegisterHook(TrainpShutdownHook)));

    IO_DEVICE com1;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1)));

    VERIFY(SUCCESSFUL(TrainpGo(&com1)));

    UCHAR speeds[NUM_TRAINS];
    RtMemset(speeds, sizeof(speeds), 0);

    for (UINT i = 0; i < sizeof(speeds); i++)
    {
        VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, i+1, 0)));
    }

    BOOLEAN running = TRUE;
    while (running)
    {
        INT senderId;
        TRAIN_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch(request.type)
        {
            case ShutdownRequest:
            {
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
                running = FALSE;
                break;
            }

            case SendDataRequest:
            {
                VERIFY(SUCCESSFUL(TrainpSendTwoByteCommand(&com1, request.train, request.speed)));
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
                break;
            }

            case SetSpeedRequest:
            {
                VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, request.train, request.speed)));
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
                speeds[request.train] = request.speed;
                break;
            }

            case ReverseRequest:
            {
                VERIFY(SUCCESSFUL(TrainpReverse(&com1, request.train)));
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                break;
            }
        }

    }

    for (UINT i = 0; i < NUM_TRAINS; i++)
    {
        if (speeds[i] != 0)
        {
            VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, i, 0)));
        }
    }

    VERIFY(SUCCESSFUL(TrainpStop(&com1)));
}

INT
TrainSendData(
        IN INT trainId,
        IN INT data
    )
{
    TRAIN_REQUEST request = { SendDataRequest, (UCHAR) trainId, (UCHAR) data };
    return TrainpSendRequest(&request);
}

INT
TrainSetSpeed(
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
TrainReverse(
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
TrainServerCreate()
{
    VERIFY(SUCCESSFUL(Create(Priority20, TrainpTask)));
}
