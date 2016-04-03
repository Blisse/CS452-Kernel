#include "calibration.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <user/trains.h>
#include <user/io.h>

#define TRAIN_NUMBER 63

static
VOID
CalibrationpSteadyStateVelocityTask()
{
    Log("Waiting for servers to initialize (10s)");
    VERIFY(SUCCESSFUL(Delay(500)));
    Log("Waiting for servers to initialize (5s)");
    VERIFY(SUCCESSFUL(Delay(500)));
    Log("Initializing calibration sequence");

    VERIFY(SUCCESSFUL(SwitchSetDirection(15, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(14, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(9, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(8, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(7, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(6, SwitchStraight)));

    UINT startTime = Time();

    while (1)
    {
        CHANGED_SENSORS changedSensors;
        VERIFY(SUCCESSFUL(SensorAwait(&changedSensors)));

        for (UINT i = 0; i < changedSensors.size; i++)
        {
            SENSOR_DATA* data = &changedSensors.sensors[i];

            if (!data->isOn)
            {
                continue;
            }

            UINT currentTime = Time();
            // Log("%c%2d|%d ", data->sensor.module, data->sensor.number, currentTime - startTime);
            startTime = currentTime;
        }
    }
}

VOID
CalibrationCreateTask()
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, CalibrationpSteadyStateVelocityTask)));
}
