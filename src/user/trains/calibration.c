#include "calibration.h"

#include <bwio/bwio.h>
#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <user/trains.h>

#define TRAIN_NUMBER 63

static
VOID
CalibrationpSteadyStateVelocityTask()
{
    // Setup the track
    // This currently assumes track B
    VERIFY(SUCCESSFUL(SwitchSetDirection(15, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(14, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(9, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(8, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(7, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(6, SwitchStraight)));

    // Wait for the track to set up
    VERIFY(SUCCESSFUL(Delay(100)));

    // Have the train go
    UINT startTime = 0;
    UINT currentSpeed = 14;
    VERIFY(SUCCESSFUL(TrainSetSpeed(TRAIN_NUMBER, currentSpeed)));

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

            SENSOR sensor = data->sensor;

            if ('C' == sensor.module && 13 == sensor.number)
            {
                startTime = Time();
            }
            else if ('E' == sensor.module && 7 == sensor.number)
            {
                UINT totalTime = Time() - startTime;
                bwprintf(BWCOM2, "%d\r\n", totalTime);

                if (currentSpeed == 5)
                {
                    bwprintf(BWCOM2, "\r\n");
                    currentSpeed = 14;
                }
                else
                {
                    currentSpeed = currentSpeed - 1;
                }

                VERIFY(SUCCESSFUL(TrainSetSpeed(TRAIN_NUMBER, currentSpeed)));
            }
        }
    }
}

VOID
CalibrationCreateTask()
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, CalibrationpSteadyStateVelocityTask)));
}
