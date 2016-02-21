#include "sensor_reader.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>

#include "display.h"

#define SENSOR_SERVER_NAME "sensor"

#define NUM_SENSORS 10
#define SENSOR_COMMAND_QUERY 0x85

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
        VERIFY(SUCCESSFUL(Delay(5)));
        VERIFY(SUCCESSFUL(WriteChar(&com1Device, SENSOR_COMMAND_QUERY)));
        VERIFY(SUCCESSFUL(Read(&com1Device, sensors, sizeof(sensors))));
        ShowSensorState(sensors, sizeof(sensors));
    }
}

VOID
SensorReaderCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority18, SensorReaderpTask)));
}
