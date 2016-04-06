#include "input_parser.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/stdlib.h>
#include <rtosc/string.h>
#include <user/trains.h>
#include <user/io.h>

static
BOOLEAN
InputParserpGetSwitchDirection (
        IN CHAR c,
        OUT SWITCH_DIRECTION* direction
    )
{
    switch (c)
    {
        case 'S':
        case 's':
            *direction = SwitchStraight;
            return TRUE;

        case 'C':
        case 'c':
            *direction = SwitchCurved;
            return TRUE;

        default:
            return FALSE;
    }
}

static
VOID
InputParserpParseCommand (
        IN STRING buffer,
        IN INT bufferLength
    )
{
    CHAR token[12];
    INT read = RtStrConsumeToken(&buffer, token, sizeof(token));
    if (read == 0)
    {
        return;
    }

    if (RtStrEqual(token, "tr"))
    {
        INT trainId;
        INT trainSpeed;

        read = RtStrScanFormatted(buffer, "%d %d", &trainId, &trainSpeed);

        if (SUCCESSFUL(read))
        {
            if (!SUCCESSFUL(TrainSetSpeed(trainId, trainSpeed)))
            {
                Log("Failed to set train %d speed %d", trainId, trainSpeed);
            }
        }
    }
    else if (RtStrEqual(token, "sw"))
    {
        INT switchId;
        CHAR switchDirection;

        read = RtStrScanFormatted(buffer, "%d %c", &switchId, &switchDirection);

        if (SUCCESSFUL(read))
        {
            SWITCH_DIRECTION direction = SwitchCurved;
            InputParserpGetSwitchDirection(switchDirection, &direction);

            if (!SUCCESSFUL(SwitchSetDirection(switchId, direction)))
            {
                Log("Failed to switch %d to %c", switchId, switchDirection);
            }
        }
    }
    else if (RtStrEqual(token, "rv"))
    {
        INT trainId;

        read = RtStrScanFormatted(buffer, "%d", &trainId);

        if (SUCCESSFUL(read))
        {
            if (!SUCCESSFUL(TrainReverse(trainId)))
            {
               Log("Failed to reverse train %d", trainId);
            }
        }
    }
    else if (RtStrEqual(token, "rs"))
    {
        INT trainId;
        SENSOR sensor;

        read = RtStrScanFormatted(buffer, "%d %c %d", &trainId, &sensor.module, &sensor.number);

        if (SUCCESSFUL(read))
        {
            TRACK_NODE* sensorNode;
            if (SUCCESSFUL(GetSensorNode(&sensor, &sensorNode)))
            {
                if (!SUCCESSFUL(ReserveTrack(sensorNode, trainId)))
                {
                    Log("Failed to reserve %s for train %d", sensorNode->name, trainId);
                }
            }
        }
    }
    else if (RtStrEqual(token, "rl"))
    {
        INT trainId;
        SENSOR sensor;

        read = RtStrScanFormatted(buffer, "%d %c %d", &trainId, &sensor.module, &sensor.number);

        if (SUCCESSFUL(read))
        {
            TRACK_NODE* sensorNode;
            if (SUCCESSFUL(GetSensorNode(&sensor, &sensorNode)))
            {
                if (!SUCCESSFUL(ReleaseTrack(sensorNode, trainId)))
                {
                    Log("Failed to release %s for train %d", sensorNode->name, trainId);
                }
            }
        }
    }
    else if (RtStrEqual(token, "goto"))
    {
        INT trainId;
        CHAR sensorModule;
        INT sensorNumber;
        INT distanceFromSensor;

        read = RtStrScanFormatted(buffer, "%d %c %d %d", &trainId, &sensorModule, &sensorNumber, &distanceFromSensor);

        if (SUCCESSFUL(read))
        {
            SENSOR sensor = { sensorModule, sensorNumber };
            if (!SUCCESSFUL(MoveTrainToSensor(trainId, sensor, distanceFromSensor)))
            {
                Log("Failed to send train %d to sensor %c%d - %d", trainId, sensorModule, sensorNumber, distanceFromSensor);
            }
        }
    }
    else if (RtStrEqual(token, "stop"))
    {
        INT trainId;

        read = RtStrScanFormatted(buffer, "%d", &trainId);

        if (SUCCESSFUL(read))
        {
            if (!SUCCESSFUL(StopTrain(trainId)))
            {
               Log("Failed to stop train %d", trainId);
            }
        }
    }
    else if (RtStrEqual(token, "start"))
    {
        INT trainId;

        read = RtStrScanFormatted(buffer, "%d", &trainId);

        if (SUCCESSFUL(read))
        {
            if (!SUCCESSFUL(StartTrain(trainId)))
            {
               Log("Failed to start train %d", trainId);
            }
        }
    }
    else if (RtStrEqual(token, "q"))
    {
        if (RtStrIsWhitespace(buffer))
        {
            Shutdown();
        }
    }
}

static
VOID
InputParserpTask()
{
    IO_DEVICE com2Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom2, &com2Device)));

    CHAR buffer[256];

    INT i;
    while (1)
    {
        for (i = 0; i < sizeof(buffer); i++)
        {
            buffer[i] = '\0';
        }

        i = 0;
        while (i < sizeof(buffer))
        {
            CHAR c = ReadChar(&com2Device);

            ShowKeyboardChar(c);

            if (c == '\r')
            {
                break;
            }

            if (c == '\b')
            {
                if (i > 0)
                {
                    buffer[--i] = '\0';
                }
            }
            else
            {
                buffer[i++] = c;
            }

        }

        InputParserpParseCommand(buffer, i);
    }
}

VOID
InputParserCreateTask()
{
    VERIFY(SUCCESSFUL(Create(Priority9, InputParserpTask)));
}
