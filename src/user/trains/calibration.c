#include "calibration.h"

#include <bwio/bwio.h>
#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <user/trains.h>

#include "display.h"

#define TRAIN_NUMBER 63

static
VOID
CalibrationpSteadyStateVelocityTask
    (
        VOID
    )
{
    // Setup the track
    // This currently assumes track B
    VERIFY(SUCCESSFUL(SwitchSetDirection(17, SwitchStraight)));
    VERIFY(SUCCESSFUL(SwitchSetDirection(13, SwitchStraight)));

    // Wait for the track to set up
    VERIFY(SUCCESSFUL(Delay(100)));

    // Have the train go
    UINT startTime = 0;
    UINT currentSpeed = 14;

    VERIFY(SUCCESSFUL(TrainSetSpeed(TRAIN_NUMBER, currentSpeed)));

    while(1)
    {
        CHANGED_SENSORS changedSensors;
        VERIFY(SUCCESSFUL(SensorAwait(&changedSensors)));

        for(UINT i = 0; i < changedSensors.size; i++)
        {
            SENSOR_DATA* data = &changedSensors.sensors[i];

            if(!data->isOn)
            {
                continue;
            }

            SENSOR sensor = data->sensor;

            if('A' == sensor.module && 13 == sensor.number)
            {
                startTime = Time();
                Log("S %d: %d", currentSpeed, startTime);

                VERIFY(SUCCESSFUL(TrainSetSpeed(63, currentSpeed)));

                if (currentSpeed == 5)
                {
                    currentSpeed = 14;
                }
                else
                {
                    currentSpeed -= 1;
                }

            }
            else if('A' == sensor.module && 1 == sensor.number)
            {
                startTime = Time();
                currentSpeed = 14;
                Log("S %d: %d", currentSpeed, startTime);

                VERIFY(SUCCESSFUL(TrainSetSpeed(63, currentSpeed)));
            }
            else if ('C' == sensor.module && 11 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("C11: %d", currentTime);
            }
            else if ('B' == sensor.module && 5 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("B5: %d", currentTime);
            }
            else if ('D' == sensor.module && 3 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("D3: %d", currentTime);
            }
            else if('E' == sensor.module && 5 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("E5: %d", currentTime);
            }
            else if('D' == sensor.module && 6 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("D6: %d", currentTime);
            }
            else if('E' == sensor.module && 10 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("E10: %d", currentTime);
            }
            else if('E' == sensor.module && 13 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("E13: %d", currentTime);
            }
            else if('D' == sensor.module && 13 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("D13: %d", currentTime);
            }
            else if('B' == sensor.module && 2 == sensor.number)
            {
                UINT currentTime = Time() - startTime;
                Log("B2 stop: %d", currentTime);
                VERIFY(SUCCESSFUL(TrainSetSpeed(63, 0)));
            }
        }
    }
}

VOID
CalibrationCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(LowestUserPriority, CalibrationpSteadyStateVelocityTask)));
}
