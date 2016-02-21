#include "sensor_reader.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>

#include "display.h"

#define SENSOR_SERVER_NAME "sensor"

#define NUM_SENSORS 10
#define SENSOR_COMMAND_QUERY 0x85

static
INT
SensorReaderpSendSensorCommand
    (
        IN IO_DEVICE* device
    )
{
    return WriteChar(device, SENSOR_COMMAND_QUERY);
}

static
VOID
SensorReaderpTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(SENSOR_SERVER_NAME)));

    CHAR sensors[NUM_SENSORS];

    IO_DEVICE com1Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1Device)));

    while (1)
    {
        Delay(5);
        SensorReaderpSendSensorCommand(&com1Device);

        UINT i;
        for (i = 0; i < NUM_SENSORS; i++)
        {
            sensors[i] = ReadChar(&com1Device);
            ShowSensorState(i, sensors[i]);
        }
    }
}

VOID
SensorReaderCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority15, SensorReaderpTask)));
}
