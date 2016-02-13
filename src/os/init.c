#include <rtos.h>
#include <rtkernel.h>

#include <bwio/bwio.h>
#include <rtosc/assert.h>
#include "clock_server.h"
#include "idle.h"
#include "io.h"
#include "name_server.h"
#include "shutdown.h"
#include "uart.h"

#include "switch_server.h"
#include "train_server.h"
#include "trains.h"

#define K3_TASKS 21

static
VOID
UserPerformanceTask
    (
        VOID
    )
{
    DelayUntil(1500);

    bwprintf(BWCOM2, "\r\n");
    bwprintf(BWCOM2, "--PERFORMANCE--\r\n");
    bwprintf(BWCOM2, "TASK\t%%active\r\n");

    TASK_PERFORMANCE performanceCounters[K3_TASKS];
    UINT i;
    UINT totalTime = 0;
    for (i = 0; i < K3_TASKS; i++)
    {
        QueryPerformance(i, &performanceCounters[i]);
        totalTime += performanceCounters[i].activeTicks;
    }

    for (i = 0; i < K3_TASKS; i++)
    {
        float fraction = (performanceCounters[i].activeTicks / ((float) totalTime)) * 100;
        UINT integerPart = fraction;
        UINT decimalPart = (fraction - integerPart) * 100;

        bwprintf(BWCOM2, "%d\t%u.%u%u\r\n", i, integerPart, decimalPart / 10, decimalPart % 10);
    }

    Shutdown();
}

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
InitTask
    (
        VOID
    )
{
    // Initialize RTOS
    IdleCreateTask();
    NameServerCreateTask();
    ShutdownCreateTask();
    ClockServerCreateTask();
    IoCreateTask();
    UartCreateTasks();

    // Initialize train tasks
    // TODO - This should be done in a different file
    TrainServerCreate(); // MUST be created first (turns on the train controller)
    SwitchServerCreate();

    // TODO - move this to a new home
    Create(LowestUserPriority, UserPerformanceTask);

    // TODO - take these out
    Create(LowestUserPriority, TestEchoTask);
    Create(LowestUserPriority, TestTrainTask);
}
