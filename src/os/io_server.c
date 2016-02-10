#include "io.h"

#include <rtosc/assert.h>

#define IO_SERVER_NAME "io"

typedef struct _IO_REGISTER_REQUEST
{
    IO_DEVICE_TYPE type;
    IO_OPEN_FUNC open;
} IO_REGISTER_REQUEST;

typedef struct _IO_OPEN_REQUEST
{
    IO_DEVICE_TYPE type;
    IO_CHANNEL channel;
} IO_OPEN_REQUEST;

typedef enum _IO_SERVER_REQUEST_TYPE
{
    RegisterRequest = 0, 
    OpenRequest
} IO_SERVER_REQUEST_TYPE;

typedef struct _IO_SERVER_REQUEST
{
    IO_SERVER_REQUEST_TYPE type;

    union
    {
        IO_REGISTER_REQUEST registerRequest;
        IO_OPEN_REQUEST openRequest;
    };
} IO_SERVER_REQUEST;

static
VOID
IopTask
    (
        VOID
    )
{
    IO_OPEN_FUNC openFunctions[NumDeviceType] = { NULL };

    VERIFY(SUCCESSFUL(RegisterAs(IO_SERVER_NAME)));

    while(1)
    {
        IO_SERVER_REQUEST request;
        INT sender;

        VERIFY(SUCCESSFUL(Receive(&sender, &request, sizeof(request))));

        switch(request.type)
        {
            case RegisterRequest:
            {
                ASSERT(NULL == openFunctions[request.registerRequest.type]);
                openFunctions[request.registerRequest.type] = request.registerRequest.open;
                VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));
                break;
            }

            case OpenRequest:
            {
                IO_DEVICE device;
                IO_OPEN_FUNC open = openFunctions[request.openRequest.type];

                ASSERT(NULL != open);

                if(SUCCESSFUL(open(request.openRequest.channel, &device)))
                {
                    VERIFY(SUCCESSFUL(Reply(sender, &device, sizeof(device))));
                }
                else
                {
                    ASSERT(FALSE);
                }

                break;
            }

            default:
            {
                ASSERT(FALSE);
                break;
            }
        }
    }
}

VOID
IoCreateTask
    (
        VOID
    )
{
    VERIFY(SUCCESSFUL(Create(Priority28, IopTask)));
}

INT
IoRegisterDriver
    (
        IN IO_DEVICE_TYPE type, 
        IN IO_OPEN_FUNC openFunc
    )
{
    INT result = WhoIs(IO_SERVER_NAME);

    if(SUCCESSFUL(result))
    {
        INT ioServerId = result;
        IO_SERVER_REQUEST request;

        request.type = RegisterRequest;
        request.registerRequest.type = type;
        request.registerRequest.open = openFunc;

        result = Send(ioServerId, 
                      &request, 
                      sizeof(request), 
                      NULL, 
                      0);
    }
    
    return result;
}

INT
Open
    (
        IN IO_DEVICE_TYPE type, 
        IN IO_CHANNEL channel, 
        OUT IO_DEVICE* device
    )
{
    INT result = WhoIs(IO_SERVER_NAME);

    if(SUCCESSFUL(result))
    {
        INT ioServerId = result;
        IO_SERVER_REQUEST request;

        request.type = OpenRequest;
        request.openRequest.type = type;
        request.openRequest.channel = channel;

        result = Send(ioServerId, 
                      &request, 
                      sizeof(request), 
                      device, 
                      sizeof(*device));
    }
    
    return result;
}
