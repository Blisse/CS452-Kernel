#include "input_parser.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/stdlib.h>
#include <rtosc/string.h>
#include <user/trains.h>

#include "display.h"

static
BOOLEAN
InputParserpGetSwitchDirection
    (
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
            break;
        case 'C':
        case 'c':
            *direction = SwitchCurved;
            return TRUE;
            break;
        default:
            break;
    }
    return FALSE;
}

static
VOID
InputParserpParseCommand
    (
        IN STRING buffer,
        IN INT bufferLength
    )
{
    CHAR arg1Buffer[12];
    CHAR arg2Buffer[12];

    INT arg1;
    INT arg2;

    CHAR token[12];
    INT read = RtStrConsumeToken(&buffer, token, sizeof(token));

    if (read == 0)
    {
        return;
    }

    if (RtStrEqual(token, "tr"))
    {
        read = RtStrConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (read && RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            read = RtStrConsumeToken(&buffer, arg2Buffer, sizeof(arg2Buffer));
            if (read && RT_SUCCESS(RtAtoi(arg2Buffer, &arg2)))
            {
                if (RtStrIsWhitespace(buffer))
                {
                    TrainSetSpeed(arg1, arg2);
                }
            }
        }
    }
    else if (RtStrEqual(token, "sw"))
    {
        read = RtStrConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (read && RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            SWITCH_DIRECTION direction;
            read = RtStrConsumeToken(&buffer, arg2Buffer, sizeof(arg2Buffer));
            if (read == 1 && InputParserpGetSwitchDirection(arg2Buffer[0], &direction))
            {
                if (RtStrIsWhitespace(buffer))
                {
                    SwitchSetDirection(arg1, direction);
                }
            }
        }
    }
    else if (RtStrEqual(token, "rv"))
    {
        read = RtStrConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (read && RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            if (RtStrIsWhitespace(buffer))
            {
                TrainReverse(arg1);
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
InputParserpTask
    (
        VOID
    )
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
InputParserCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority24, InputParserpTask)));
}
