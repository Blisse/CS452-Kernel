#include "sensor_server.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>

#include "display.h"

#include <bwio/bwio.h>
#include <user/trains.h>

#define SENSOR_SERVER_NAME "sensor"

#define NUM_SENSORS 10
#define SENSOR_COMMAND_QUERY 0x85

static
VOID
SensorServerpTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(SENSOR_SERVER_NAME)));

    CHAR sensors[NUM_SENSORS];
    UINT startTime = 0;
    UINT currentSpeed = 14;
    BOOLEAN tripped = FALSE;

    IO_DEVICE com1Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1Device)));

    VERIFY(SUCCESSFUL(SwitchSetDirection(15, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(14, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(9, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(8, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(7, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(6, SwitchStraight)));

    VERIFY(SUCCESSFUL(Delay(100)));
    VERIFY(SUCCESSFUL(TrainSetSpeed(63, currentSpeed)));

    while (1)
    {
        VERIFY(SUCCESSFUL(WriteChar(&com1Device, SENSOR_COMMAND_QUERY)));
        VERIFY(SUCCESSFUL(Read(&com1Device, sensors, sizeof(sensors))));
        ShowSensorState(sensors, sizeof(sensors));

        if(sensors[5] & (1 << 3)) 
        {
            startTime = Time();
        }

        if(sensors[8] & (1 << 1) && !tripped)
        {
            UINT totalTime = Time() - startTime;
            bwprintf(BWCOM2, "%d\r\n", totalTime);
            tripped = TRUE;
            VERIFY(SUCCESSFUL(Delay(100)));

            if(currentSpeed == 4)
            {
                bwprintf(BWCOM2, "\r\n");
                currentSpeed = 14;
            }
            else
            {
                currentSpeed = currentSpeed - 1;
            }

            VERIFY(SUCCESSFUL(TrainSetSpeed(63, currentSpeed)));
        }
        else
        {
            tripped = FALSE;
        }
    }
}

VOID
SensorServerCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority18, SensorServerpTask)));
}
