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
        CHAR arg1Buffer[12];
        INT arg1;

        read = RtStrConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (read && RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            CHAR arg2Buffer[12];
            INT arg2;

            read = RtStrConsumeToken(&buffer, arg2Buffer, sizeof(arg2Buffer));
            if (read && RT_SUCCESS(RtAtoi(arg2Buffer, &arg2)))
            {
                if (RtStrIsWhitespace(buffer))
                {
                    if (!SUCCESSFUL(TrainSetSpeed(arg1, arg2)))
                    {
                        Log("Failed to set train %d speed %d", arg1, arg2);
                    }
                }
            }
        }
    }
    else if (RtStrEqual(token, "sw"))
    {
        CHAR arg1Buffer[12];
        INT arg1;

        read = RtStrConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (read && RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            CHAR arg2Buffer[12];
            SWITCH_DIRECTION direction;

            read = RtStrConsumeToken(&buffer, arg2Buffer, sizeof(arg2Buffer));
            if (read == 1 && InputParserpGetSwitchDirection(arg2Buffer[0], &direction))
            {
                if (RtStrIsWhitespace(buffer))
                {
                    if (!SUCCESSFUL(SwitchSetDirection(arg1, direction)))
                    {
                        Log("Failed to switch %d to %c", arg1, arg2Buffer[0]);
                    }
                }
            }
        }
    }
    else if (RtStrEqual(token, "rv"))
    {
        CHAR arg1Buffer[12];
        INT arg1;

        read = RtStrConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (read && RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            if (RtStrIsWhitespace(buffer))
            {
                if (!SUCCESSFUL(TrainReverse(arg1)))
                {
                   Log("Failed to reverse train %d", arg1);
                }
            }
        }
    }
    else if (RtStrEqual(token, "goto"))
    {
        CHAR arg1Buffer[12];
        INT arg1;

        read = RtStrConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (read && RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            CHAR arg2Buffer[12];
            CHAR arg2;

            read = RtStrConsumeToken(&buffer, arg2Buffer, sizeof(arg2Buffer));
            if (read == 1 && (arg2 = arg2Buffer[0]))
            {
                CHAR arg3Buffer[12];
                INT arg3;

                read = RtStrConsumeToken(&buffer, arg3Buffer, sizeof(arg3Buffer));
                if (read && RT_SUCCESS(RtAtoi(arg3Buffer, &arg3)))
                {
                    CHAR arg4Buffer[12];
                    INT arg4;

                    read = RtStrConsumeToken(&buffer, arg4Buffer, sizeof(arg4Buffer));
                    if (read && RT_SUCCESS(RtAtoi(arg4Buffer, &arg4)))
                    {
                        if (RtStrIsWhitespace(buffer))
                        {
                            SENSOR sensor = { arg2, arg3 };
                            SchedulerMoveTrainToSensor(arg1, sensor, arg4);
                        }
                    }
                }
            }
        }
    }
    else if (RtStrEqual(token, "stop"))
    {
        CHAR arg1Buffer[12];
        INT arg1;

        read = RtStrConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (read && RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            SchedulerStopTrain(arg1);
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
