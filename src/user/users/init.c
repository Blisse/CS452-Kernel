#include <user/users.h>

#include <rtosc/assert.h>
#include <user/trains.h>
#include <rtkernel.h>
#include <rtos.h>

static
VOID
TestEchoTask
    (
        VOID
    )
{
    IO_DEVICE device;

    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom2, &device)));

    while(1)
    {
        // This is just a demonstration of how the i/o server tries to be efficient
        // You can wait for a whole bunch of characters to be ready
        // This should echo in batches of 2, instead of every character
        CHAR buffer[2];

        VERIFY(SUCCESSFUL(Read(&device, buffer, sizeof(buffer))));
        VERIFY(SUCCESSFUL(Write(&device, buffer, sizeof(buffer))));
    }
}

static
VOID
TestTrainTask
    (
        VOID
    )
{
    // Wait for the switch server to configure the track
    VERIFY(SUCCESSFUL(Delay(500)));

    // Have a train start moving
    VERIFY(SUCCESSFUL(TrainSetSpeed(68, 11)));

    // Wait for a bit, then reverse the train's direction
    VERIFY(SUCCESSFUL(Delay(400)));
    VERIFY(SUCCESSFUL(TrainReverse(68)));

    // Try moving a switch near the operator's chair
    VERIFY(SUCCESSFUL(SwitchSetDirection(156, SwitchStraight)));
}

VOID
InitUserTasks
    (
        VOID
    )
{
    VERIFY(RT_SUCCESS(Create(HighestUserPriority, InitTrainTasks)));
    VERIFY(RT_SUCCESS(Create(LowestUserPriority, TestEchoTask)));
    VERIFY(RT_SUCCESS(Create(LowestUserPriority, TestTrainTask)));
}
