#include "train_server.h"

#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>

#include "location_server.h"

#define TRAIN_SERVER_NAME "train"
#define NUM_TRAINS 80

#define TRAIN_COMMAND_REVERSE 0xF
#define TRAIN_COMMAND_GO 0x60
#define TRAIN_COMMAND_STOP 0x61

typedef enum _TRAIN_REQUEST_TYPE {
    ShutdownRequest = 0,
    SetSpeedRequest,
    GetSpeedRequest,
    ReverseRequest
} TRAIN_REQUEST_TYPE;

typedef struct _TRAIN_REQUEST {
    TRAIN_REQUEST_TYPE type;
    UCHAR train;
    UCHAR speed;
    UCHAR* trainSpeed;
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

    for (UINT i = 0; i < sizeof(speeds); i++) {
        VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, i + 1, 0)));
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
                running = FALSE;
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
                break;

            case GetSpeedRequest:
                *request.trainSpeed = speeds[request.train - 1];
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
                break;

            case SetSpeedRequest:
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, request.train, request.speed)));
                VERIFY(SUCCESSFUL(LocationServerUpdateTrainSpeed(request.train, request.speed)));

                speeds[request.train - 1] = request.speed;
                break;

            case ReverseRequest:
            {
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                UCHAR oldSpeed = speeds[request.train - 1];

                VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, request.train, 0)));
                VERIFY(SUCCESSFUL(LocationServerUpdateTrainSpeed(request.train, 0)));

                Delay(100 * (oldSpeed / 3 + 1));

                VERIFY(SUCCESSFUL(TrainpReverse(&com1, request.train)));
                VERIFY(SUCCESSFUL(LocationServerTrainDirectionReverse(request.train)));

                VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, request.train, oldSpeed)));
                VERIFY(SUCCESSFUL(LocationServerUpdateTrainSpeed(request.train, oldSpeed)));

                break;
            }
        }

    }

    for (UINT i = 0; i < NUM_TRAINS; i++)
    {
        if (speeds[i] != 0)
        {
            VERIFY(SUCCESSFUL(TrainpSetSpeed(&com1, i + 1, 0)));
        }
    }

    VERIFY(SUCCESSFUL(TrainpStop(&com1)));
}

INT
TrainGetSpeed(
        IN INT train,
        OUT UCHAR* speed
    )
{
    if (!(0 <= train && train < NUM_TRAINS))
    {
        return -1;
    }

    TRAIN_REQUEST request = { GetSpeedRequest, train, 0, speed };
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
