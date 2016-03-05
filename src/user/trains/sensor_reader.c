#include "sensor_reader.h"

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
    SENSOR_DATA data;
} SENSOR_SERVER_REQUEST;

static
INT
SensorReaderpSendMessage
    (
        SENSOR_SERVER_REQUEST request
    )
{
    INT sensorServerId = WhoIs(SENSOR_SERVER_NAME);

    if (SUCCESSFUL(sensorServerId))
    {
        VERIFY(SUCCESSFUL(Send(sensorServerId, &request, sizeof(request), NULL, 0)));
    }

    return sensorServerId;
}

static
VOID
SensorReaderpUpdate
    (
        IN UCHAR* previousSensors,
        IN UCHAR* currentSensors,
        IN INT sensorsLength
    )
{
    for (UINT i = 0; i < sensorsLength; i++)
    {
        UCHAR previousSensorValue = previousSensors[i];
        UCHAR currentSensorValue = currentSensors[i];

        for (UINT j = 0; j < 8; j++)
        {
            INT previousValue = BIT_CHECK(previousSensorValue, j);
            INT currentValue = BIT_CHECK(currentSensorValue, j);

            if (previousValue || previousValue != currentValue)
            {
                CHAR module = 'A' + (i / 2);
                UINT number = (8 - j) + ((i % 2) * 8);
                UINT status = currentValue;

                SENSOR_DATA data = { module, number, status };
                ShowSensorStatus(data);

                SENSOR_SERVER_REQUEST request = { DataRequest, data };
                SensorReaderpSendMessage(request);
            }
        }

        previousSensors[i] = currentSensors[i];
    }
}

static
VOID
SensorReaderpTask
    (
        VOID
    )
{
    UCHAR previousSensors[NUM_SENSORS];
    RtMemset(previousSensors, sizeof(previousSensors));

    UCHAR currentSensors[NUM_SENSORS];
    RtMemset(currentSensors, sizeof(currentSensors));

    IO_DEVICE com1Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1Device)));

    while (1)
    {
        VERIFY(SUCCESSFUL(Delay(5)));
        VERIFY(SUCCESSFUL(WriteChar(&com1Device, SENSOR_COMMAND_QUERY)));
        VERIFY(SUCCESSFUL(Read(&com1Device, currentSensors, sizeof(currentSensors))));
        SensorReaderpUpdate(previousSensors, currentSensors, sizeof(previousSensors));
    }
}

static
VOID
SensorServerpTask
    (
        VOID
    )
{
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
                INT subscriberId = senderId;
                VERIFY(RT_SUCCESS(RtCircularBufferPush(&subscriberBuffer, &subscriberId, sizeof(subscriberId))));
                break;
            }
            case DataRequest:
            {
                INT subscriberId;
                while (RT_SUCCESS(RtCircularBufferPeekAndPop(&subscriberBuffer, &subscriberId, sizeof(subscriberId))))
                {
                    SENSOR_DATA data = request.data;
                    VERIFY(SUCCESSFUL(Send(subscriberId, &data, sizeof(data), NULL, 0)));
                }
                break;
            }
        }

        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
    }
}

VOID
SensorReaderCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority17, SensorServerpTask)));
    VERIFY(SUCCESSFUL(Create(Priority18, SensorReaderpTask)));
}

VOID
SensorDataRegister
    (
        VOID
    )
{
    SENSOR_SERVER_REQUEST request = { DataRequest };
    VERIFY(SUCCESSFUL(SensorReaderpSendMessage(request)));
}
