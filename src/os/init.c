#include "init.h"

#include <bwio/bwio.h>
#include <rtosc/assert.h>
#include "clock_server.h"
#include "idle.h"
#include "io.h"
#include "name_server.h"
#include "shutdown.h"
#include "uart.h"

#define K3_TASKS 14

static
VOID
UserPerformanceTask
    (
        VOID
    )
{
    DelayUntil(300);

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
TestTrainTaskShutdown
    (
        VOID
    )
{
    IO_DEVICE com1;
    bwprintf(BWCOM2, "S!\r\n");
    // Open handles to com1
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1)));

    // Turn off the train
    VERIFY(SUCCESSFUL(WriteChar(&com1, 0x61)));
}

static
VOID
TestTrainTask
    (
        VOID
    )
{
    IO_DEVICE com1;
    IO_DEVICE com2;

    // Open handles to com1 and com2
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1)));
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom2, &com2)));

    // Turn on the train
    VERIFY(SUCCESSFUL(WriteChar(&com1, 0x60)));

    // Register a shutdown hook
    VERIFY(SUCCESSFUL(ShutdownRegisterHook(TestTrainTaskShutdown)));

    while(1)
    {
        CHAR sensorData[10];

        // Grab sensor data
        VERIFY(SUCCESSFUL(WriteChar(&com1, 0x85)));
        VERIFY(SUCCESSFUL(Read(&com1, sensorData, sizeof(sensorData))));

        // Display the sensor data
        VERIFY(SUCCESSFUL(WriteString(&com2, "Sensor: \r\n")));

        // Sensors can only change every so often
        VERIFY(SUCCESSFUL(Delay(5)));
    }
}

VOID
InitTask
    (
        VOID
    )
{
    IdleCreateTask();
    NameServerCreateTask();
    ShutdownCreateTask();
    ClockServerCreateTask();
    IoCreateTask();
    UartCreateTasks();
    Create(LowestUserPriority, UserPerformanceTask);
    Create(HighestUserPriority, TestEchoTask);
    Create(HighestUserPriority, TestTrainTask);
}
