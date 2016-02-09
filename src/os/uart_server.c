#include "uart_server.h"

#include <rtosc/assert.h>
#include <rtosc/string.h>

#include "name_server.h"
#include "rtos.h"

#define UART_COM1_INPUT_NAME "c1_in"
#define UART_COM1_OUTPUT_NAME "c1_out"
#define UART_COM2_INPUT_NAME "c2_in"
#define UART_COM2_OUTPUT_NAME "c2_out"

typedef enum _UART_SERVER_REQUEST_TYPE
{
    PutRequest = 0,
    GetRequest
} UART_SERVER_REQUEST_TYPE;

typedef struct _UART_SERVER_REQUEST
{
    UART_SERVER_REQUEST_TYPE type;
    PVOID buffer;
    UINT bufferLength;
} UART_SERVER_REQUEST;

static
VOID
UartServerpCom1InputTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(UART_COM1_INPUT_NAME)));
}

static
VOID
UartServerpCom1OutputTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(UART_COM1_OUTPUT_NAME)));
}

static
VOID
UartServerpCom2InputTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(UART_COM2_INPUT_NAME)));
}

static
VOID
UartServerpCom2OutputTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(RegisterAs(UART_COM2_OUTPUT_NAME)));
}

VOID
UartServerCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority28, UartServerpCom1InputTask)));
    VERIFY(SUCCESSFUL(Create(Priority28, UartServerpCom1OutputTask)));
    VERIFY(SUCCESSFUL(Create(Priority28, UartServerpCom2InputTask)));
    VERIFY(SUCCESSFUL(Create(Priority28, UartServerpCom2OutputTask)));
}

static
inline
INT
UartServerpGetChannelTaskId
    (
        IN INT channel, 
        IN UART_SERVER_REQUEST_TYPE type
    )
{
    if(COM1_CHANNEL == channel)
    {
        if(PutRequest == type)
        {
            return WhoIs(UART_COM1_OUTPUT_NAME);
        }
        else if(GetRequest == type)
        {
            return WhoIs(UART_COM1_INPUT_NAME);
        }
        else
        {
            ASSERT(FALSE);
            return 0;
        }
    }
    else if(COM2_CHANNEL == channel)
    {
        if(PutRequest == type)
        {
            return WhoIs(UART_COM2_OUTPUT_NAME);
        }
        else if(GetRequest == type)
        {
            return WhoIs(UART_COM2_INPUT_NAME);
        }
        else
        {
            ASSERT(FALSE);
            return 0;
        }
    }
    else
    {
        ASSERT(FALSE);
        return 0;
    }
}

static
inline
INT
UartServerpSendToChannel
    (
        IN INT channel, 
        IN UART_SERVER_REQUEST* request
    )
{
    INT response; 
    INT targetTask = UartServerpGetChannelTaskId(channel, request->type);
    ASSERT(SUCCESSFUL(targetTask));

    VERIFY(SUCCESSFUL(Send(targetTask, 
                           request, 
                           sizeof(*request), 
                           &response, 
                           sizeof(response))));

    return response;
}

INT
GetString
    (
        IN INT channel, 
        IN STRING buffer, 
        IN UINT bufferLength
    )
{
    UART_SERVER_REQUEST request = { GetRequest, buffer, bufferLength };

    return UartServerpSendToChannel(channel, &request);
}

INT
PutString
    (
        IN INT channel, 
        IN STRING str
    )
{
    UART_SERVER_REQUEST request = { PutRequest, str, RtStrLen(str) };

    return UartServerpSendToChannel(channel, &request);
}

INT
PutFormattedString
    (
        IN INT channel, 
        IN STRING str
    )
{
    // TODO
    // Do we need this?
    ASSERT(FALSE);
    return -1;
}

CHAR
Getc
    (
        IN INT channel
    )
{
    CHAR c;
    UART_SERVER_REQUEST request = { GetRequest, &c, sizeof(c) };

    return UartServerpSendToChannel(channel, &request);
}

INT
Putc
    (
        IN INT channel, 
        IN CHAR c
    )
{
    UART_SERVER_REQUEST request = { PutRequest, &c, sizeof(c) };

    return UartServerpSendToChannel(channel, &request);
}
