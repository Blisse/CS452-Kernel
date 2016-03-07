#include "sensor_server.h"

#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>
#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rtosc/bitset.h>
#include <rtosc/string.h>

#include "display.h"

#define SENSOR_SERVER_NAME "sensor"

#define NUM_SENSORS 10
#define SENSOR_COMMAND_QUERY 0x85

typedef enum _SENSOR_SERVER_REQUEST_TYPE
{
    RegisterRequest = 0,
    DataRequest
} SENSOR_SERVER_REQUEST_TYPE;

typedef struct _SENSOR_SERVER_REQUEST
{
    SENSOR_SERVER_REQUEST_TYPE type;
    CHANGED_SENSORS* changedSensors;
} SENSOR_SERVER_REQUEST;

static
VOID
SensorReaderpUpdate
    (
        IN INT sensorServerId, 
        IN UCHAR* previousSensors,
        IN UCHAR* currentSensors,
        IN INT sensorsLength
    )
{
    // Keep track of which sensors have changed
    CHANGED_SENSORS changedSensors;
    changedSensors.size = 0;

    // Go through each module
    for (UINT i = 0; i < sensorsLength; i++)
    {
        UCHAR previousSensorValue = previousSensors[i];
        UCHAR currentSensorValue = currentSensors[i];

        // Go through each sensor in this module
        for (UINT j = 0; j < 8; j++)
        {
            BOOLEAN previousValue = BIT_CHECK(previousSensorValue, j);
            BOOLEAN currentValue = BIT_CHECK(currentSensorValue, j);

            // Check to see if the sensor has changed
            if (previousValue != currentValue)
            {
                // Make sure there is still room for this sensor
                if(changedSensors.size < MAX_TRACKABLE_TRAINS)
                {
                    // Add this sensor to the list of changed sensors
                    SENSOR_DATA* data = &changedSensors.sensors[changedSensors.size];

                    data->sensor.module = 'A' + (i / 2);
                    data->sensor.number = (8 - j) + ((i % 2) * 8);
                    data->isOn = currentValue;

                    changedSensors.size++;

                    // TODO: What if we had the display server register for sensor updates
                    //       the same as everyone else?
                    ShowSensorStatus(*data);
                }
                else
                {
                    ASSERT(FALSE);
                }
            }
        }

        // Remember the sensor's value
        previousSensors[i] = currentSensors[i];
    }

    // If some sensors have changed, then update the sensor server
    if (changedSensors.size > 0)
    {
        SENSOR_SERVER_REQUEST request = { DataRequest, &changedSensors };
        VERIFY(SUCCESSFUL(Send(sensorServerId, &request, sizeof(request), NULL, 0)));
    }
}

static
VOID
SensorReaderpTask
    (
        VOID
    )
{
    INT sensorServerId = MyParentTid();
    ASSERT(SUCCESSFUL(sensorServerId));

    UCHAR previousSensors[NUM_SENSORS];
    RtMemset(previousSensors, sizeof(previousSensors), 0);

    UCHAR currentSensors[NUM_SENSORS];
    RtMemset(currentSensors, sizeof(currentSensors), 0);

    IO_DEVICE com1Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1Device)));

    while (1)
    {
        VERIFY(SUCCESSFUL(WriteChar(&com1Device, SENSOR_COMMAND_QUERY)));
        VERIFY(SUCCESSFUL(Read(&com1Device, currentSensors, sizeof(currentSensors))));
        SensorReaderpUpdate(sensorServerId, previousSensors, currentSensors, sizeof(previousSensors));
    }
}

static
VOID
SensorServerpTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority18, SensorReaderpTask)));
    VERIFY(SUCCESSFUL(RegisterAs(SENSOR_SERVER_NAME)));

    INT underlyingSubscriberBuffer[NUM_TASKS];
    RT_CIRCULAR_BUFFER subscriberBuffer;
    RtCircularBufferInit(&subscriberBuffer, underlyingSubscriberBuffer, sizeof(underlyingSubscriberBuffer));

    while (1)
    {
        INT senderId;
        SENSOR_SERVER_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch (request.type)
        {
            case RegisterRequest:
            {
                VERIFY(RT_SUCCESS(RtCircularBufferPush(&subscriberBuffer, &senderId, sizeof(senderId))));
                break;
            }

            case DataRequest:
            {
                INT subscriberId;

                while (RT_SUCCESS(RtCircularBufferPeekAndPop(&subscriberBuffer, &subscriberId, sizeof(subscriberId))))
                {
                    VERIFY(SUCCESSFUL(Reply(subscriberId, request.changedSensors, sizeof(*request.changedSensors))));
                }

                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));             

                break;
            }
        }
    }
}

VOID
SensorServerCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority17, SensorServerpTask)));
}

INT
SensorAwait
    (
        OUT CHANGED_SENSORS* changedSensors
    )
{
    INT result = WhoIs(SENSOR_SERVER_NAME);

    if(SUCCESSFUL(result))
    {
        SENSOR_SERVER_REQUEST request = { RegisterRequest };
        INT sensorServerId = result;

        result = Send(sensorServerId, &request, sizeof(request), changedSensors, sizeof(*changedSensors));
    }

    return result;
}
