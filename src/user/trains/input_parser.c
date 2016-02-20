#include "input_parser.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/stdlib.h>
#include <rtosc/string.h>
#include <user/trains.h>

#include "display.h"

static
VOID
InputParserpConsumeToken
    (
        IN STRING* str,
        OUT STRING buffer,
        IN INT bufferLength
    )
{
    CHAR* p = *str;
    CHAR c;
    // ignore leading space
    while ((c = *p++) && isspace(c))
    {
    }

    // read until space
    UINT i = 0;
    while (!isspace(c) && (i < bufferLength))
    {
        buffer[i++] = c;
        c = *p++;
    }

    // terminate buffer
    buffer[i] = '\0';

    // consume token
    *str = p;
}

static
BOOLEAN
InputParserpIsWhitespace
    (
        IN STRING str
    )
{
    CHAR c;
    while ((c = *str++))
    {
        if (!isspace(c))
        {
            return FALSE;
        }
    }
    return TRUE;
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
    InputParserpConsumeToken(&buffer, token, sizeof(token));

    bwprintf(BWCOM2, "token : %s \r\n", token);
    bwprintf(BWCOM2, "buffer : %s \r\n", buffer);

    if (RtStrEqual(token, "tr"))
    {
        InputParserpConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            InputParserpConsumeToken(&buffer, arg2Buffer, sizeof(arg2Buffer));
            if (RT_SUCCESS(RtAtoi(arg2Buffer, &arg2)))
            {
                if (InputParserpIsWhitespace(buffer))
                {
                    TrainSetSpeed(arg1, arg2);
                }
            }
        }
    }
    else if (RtStrEqual(token, "sw"))
    {
        InputParserpConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            InputParserpConsumeToken(&buffer, arg2Buffer, sizeof(arg2Buffer));

            if ((arg2Buffer[0] == 'S'
                || arg2Buffer[0] == 's'
                || arg2Buffer[0] == 'C'
                || arg2Buffer[0] == 'c')
                && arg2Buffer[1] == '\0')
            {
                SWITCH_DIRECTION direction = SwitchCurved;
                if (arg2Buffer[0] == 'S' || arg2Buffer[0] == 's')
                {
                    direction = SwitchStraight;
                }

                if (InputParserpIsWhitespace(buffer))
                {
                    SwitchSetDirection(arg1, direction);
                }
            }
        }
    }
    else if (RtStrEqual(token, "rv"))
    {
        InputParserpConsumeToken(&buffer, arg1Buffer, sizeof(arg1Buffer));
        if (RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
        {
            if (InputParserpIsWhitespace(buffer))
            {
                TrainReverse(arg1);
            }
        }

    }
    else if (RtStrEqual(token, "q"))
    {
        if (InputParserpIsWhitespace(buffer))
        {
            Shutdown();
        }
    }
}

static
CHAR
InputParserpReadChar
    (
        IN IO_DEVICE* com2Device
    )
{
    CHAR c = ReadChar(com2Device);

    ShowKeyboardChar(c);

    return c;
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
    for (i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = '\0';
    }

    while (1)
    {
        for (i = 0; i < sizeof(buffer); i++)
        {
            CHAR c = InputParserpReadChar(&com2Device);

            if (c == '\r')
            {
                break;
            }
            else if (c == '\b')
            {
                i--;
            }
            else
            {
                buffer[i] = c;
            }
        }

        INT endIdx = min(i, 255);

        buffer[endIdx] = '\0';

        InputParserpParseCommand(buffer, endIdx);

        while (i > 0)
        {
            buffer[i--] = '\0';
        }
    }
}

VOID
InputParserCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(HighestUserPriority, InputParserpTask)));
}
