#include "input_parser.h"

#include <rtkernel.h>
#include <rtos.h>
#include <rtosc/assert.h>
#include <rtosc/stdlib.h>
#include <rtosc/string.h>
#include <user/trains.h>

static
VOID
InputParserpReadToken
    (
        IN IO_DEVICE* com2Device,
        OUT STRING buffer,
        IN INT bufferLength
    )
{
    CHAR c;
    // ignore leading space
    while ((c = ReadChar(com2Device)) && isspace(c))
    {
    }

    // read until space
    UINT i = 0;
    while (!isspace(c) && i < (bufferLength-1))
    {
        buffer[i++] = c;
        c = ReadChar(com2Device);
    }

    buffer[min(i, bufferLength)] = '\0';
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

    while (1)
    {
        CHAR buffer[12] = { 0 };

        InputParserpReadToken(&com2Device, buffer, sizeof(buffer));

        if (RtStrEqual(buffer, "tr"))
        {
            CHAR arg1Buffer[12] = { 0 };
            CHAR arg2Buffer[12] = { 0 };

            INT arg1;
            INT arg2;

            InputParserpReadToken(&com2Device, buffer, sizeof(buffer));
            if (RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
            {
                InputParserpReadToken(&com2Device, buffer, sizeof(buffer));
                if (RT_SUCCESS(RtAtoi(arg2Buffer, &arg2)))
                {
                    TrainSetSpeed(arg1, arg2);
                }
            }
        }
        else if (RtStrEqual(buffer, "sw"))
        {
            CHAR arg1Buffer[12] = { 0 };
            CHAR arg2Buffer[12] = { 0 };

            INT arg1;
            INT arg2;

            InputParserpReadToken(&com2Device, buffer, sizeof(buffer));
            if (RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
            {
                InputParserpReadToken(&com2Device, buffer, sizeof(buffer));
                if (RT_SUCCESS(RtAtoi(arg2Buffer, &arg2)))
                {
                    SwitchSetDirection(arg1, arg2);
                }
            }
        }
        else if (RtStrEqual(buffer, "rv"))
        {
            CHAR arg1Buffer[12] = { 0 };

            INT arg1;

            InputParserpReadToken(&com2Device, buffer, sizeof(buffer));
            if (RT_SUCCESS(RtAtoi(arg1Buffer, &arg1)))
            {
                TrainReverse(arg1);
            }
        }
        else if (RtStrEqual(buffer, "q"))
        {
            Shutdown();
        }
    }
}

VOID
InputParserCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority10, InputParserpTask)));
}
