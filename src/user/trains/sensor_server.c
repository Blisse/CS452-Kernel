#include "sensor_server.h"

#include <rtkernel.h>
#include <rtos.h>
#include <user/trains.h>
#include <user/io.h>
#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rtosc/bitset.h>
#include <rtosc/string.h>

#define SENSOR_SERVER_NAME "sensor"

#define NUM_SENSOR_RESPONSES 10
#define SENSOR_COMMAND_QUERY 0x85

typedef enum _SENSOR_SERVER_REQUEST_TYPE {
    SubscribeRequest = 0,
    DataRequest,
} SENSOR_SERVER_REQUEST_TYPE;

typedef struct _SENSOR_SERVER_REQUEST {
    SENSOR_SERVER_REQUEST_TYPE type;
    CHANGED_SENSORS changedSensors;
} SENSOR_SERVER_REQUEST;

static
VOID
SensorReaderpUpdate(
        IN INT sensorServerId,
        IN UCHAR* previousSensors,
        IN UCHAR* currentSensors,
        IN INT sensorsLength
    )
{
    SENSOR_SERVER_REQUEST request;
    request.type = DataRequest;
    request.changedSensors.size = 0;

    for (UINT i = 0; i < sensorsLength; i++)
    {
        UCHAR previousSensorValue = previousSensors[i];
        UCHAR currentSensorValue = currentSensors[i];

        previousSensors[i] = currentSensors[i];

        for (UINT j = 0; j < 8; j++)
        {
            BOOLEAN previousValue = BIT_CHECK(previousSensorValue, j);
            BOOLEAN currentValue = BIT_CHECK(currentSensorValue, j);

            if (previousValue != currentValue)
            {
                CHAR sensorModule = 'A' + (i / 2);
                INT sensorNumber = (8 - j) + ((i % 2) * 8);

                if (request.changedSensors.size < MAX_TRACKABLE_TRAINS)
                {
                    SENSOR_DATA* data = &request.changedSensors.sensors[request.changedSensors.size];

                    data->sensor.module = sensorModule;
                    data->sensor.number = sensorNumber;
                    data->isOn = currentValue;

                    request.changedSensors.size++;
                }
                else
                {
                    Log("Received junk sensor data from %c%d", sensorModule, sensorNumber);
                    break;
                }
            }
        }
    }

    if (request.changedSensors.size > 0)
    {
        VERIFY(SUCCESSFUL(Send(sensorServerId, &request, sizeof(request), NULL, 0)));
    }
}

static
VOID
SensorReaderpTask()
{
    INT sensorServerId = MyParentTid();
    ASSERT(SUCCESSFUL(sensorServerId));

    UCHAR previousSensors[NUM_SENSOR_RESPONSES];
    RtMemset(previousSensors, sizeof(previousSensors), 0);

    UCHAR currentSensors[NUM_SENSOR_RESPONSES];
    RtMemset(currentSensors, sizeof(currentSensors), 0);

    IO_DEVICE com1Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1Device)));

    for (INT i = 10; i >= 0; i--)
    {
        Log("Waiting for junk sensor data (%ds)", i);
        VERIFY(SUCCESSFUL(Delay(100)));
    }

    VERIFY(SUCCESSFUL(FlushInput(&com1Device)));
    Log("Flushed junk sensor data");

    while (1)
    {
        VERIFY(SUCCESSFUL(WriteChar(&com1Device, SENSOR_COMMAND_QUERY)));
        VERIFY(SUCCESSFUL(Read(&com1Device, currentSensors, sizeof(currentSensors))));
        SensorReaderpUpdate(sensorServerId, previousSensors, currentSensors, sizeof(previousSensors));
    }
}

static
VOID
SensorServerpTask()
{
    VERIFY(SUCCESSFUL(Create(HighestUserPriority, SensorReaderpTask)));
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
            case SubscribeRequest:
            {
                VERIFY(RT_SUCCESS(RtCircularBufferPush(&subscriberBuffer, &senderId, sizeof(senderId))));
                break;
            }

            case DataRequest:
            {
                VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));

                INT subscriberId;
                while (RT_SUCCESS(RtCircularBufferPeekAndPop(&subscriberBuffer, &subscriberId, sizeof(subscriberId))))
                {
                    VERIFY(SUCCESSFUL(Reply(subscriberId, &request.changedSensors, sizeof(request.changedSensors))));
                }

                break;
            }
        }
    }
}

VOID
SensorServerCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority18, SensorServerpTask)));
}

INT
SensorAwait(
        OUT CHANGED_SENSORS* changedSensors
    )
{
    INT result = WhoIs(SENSOR_SERVER_NAME);

    if (SUCCESSFUL(result))
    {
        INT sensorServerId = result;
        SENSOR_SERVER_REQUEST request;
        request.type = SubscribeRequest;

        result = Send(sensorServerId, &request, sizeof(request), changedSensors, sizeof(*changedSensors));
    }

    return result;
}
