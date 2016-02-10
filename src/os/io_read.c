#include "io.h"

#include <rtosc/assert.h>
#include "courier.h"

typedef struct _IO_READ_TASK_NOTIFIER_PARAMS
{
    EVENT event;
    IO_READ_FUNC read;
} IO_READ_TASK_NOTIFIER_PARAMS;

typedef struct _IO_READ_TASK_PARAMS
{
    IO_READ_TASK_NOTIFIER_PARAMS notifierParams;
    STRING name;
} IO_READ_TASK_PARAMS;

typedef enum _IO_READ_REQUEST_TYPE
{
    NotifierRequest = 0, 
    ReadRequest
} IO_READ_REQUEST_TYPE;

typedef struct _IO_READ_REQUEST
{
    IO_READ_REQUEST_TYPE type;
    PVOID buffer;
    UINT bufferLength;
} IO_READ_REQUEST;

static
VOID
IopReadNotifierTask
    (
        VOID
    )
{
    IO_READ_REQUEST request = { NotifierRequest };
    IO_READ_TASK_NOTIFIER_PARAMS params;
    INT parentId;

    // Get parameters
    VERIFY(SUCCESSFUL(Receive(&parentId, &params, sizeof(params))));
    VERIFY(SUCCESSFUL(Reply(parentId, NULL, 0)));

    // Setup a courier
    CourierCreateTask(Priority30, MyTid(), parentId);

    // Run the notifier
    while(1)
    {
        CHAR c;

        // Wait for the read event to come in
        VERIFY(SUCCESSFUL(AwaitEvent(params.event)));

        // Read the new character
        c = params.read();

        // Send it off to the read server
        request.buffer = &c;
        request.bufferLength = sizeof(c);
        CourierPickup(&request, sizeof(request));
    }
}

static
VOID
IopReadTask
    (
        VOID
    )
{
    IO_READ_TASK_PARAMS params;
    INT sender;
    INT notifierTaskId;

    // Get parameters
    VERIFY(SUCCESSFUL(Receive(&sender, &params, sizeof(params))));
    VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));

    // Register with the name server
    VERIFY(SUCCESSFUL(RegisterAs(params.name)));

    // Set up the notifier task
    notifierTaskId = Create(HighestSystemPriority, IopReadNotifierTask);
    ASSERT(SUCCESSFUL(notifierTaskId));

    // Send the notifier task the necessary parameters
    VERIFY(SUCCESSFUL(Send(notifierTaskId, 
                           &params.notifierParams, 
                           sizeof(params.notifierParams), 
                           NULL, 
                           0)));

    // Run the server
    while(1)
    {
        IO_READ_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&sender, &request, sizeof(request))));

        switch(request.type)
        {
            case NotifierRequest:
                ASSERT(FALSE);
                break;

            case ReadRequest:
                ASSERT(FALSE);
                break;

            default:
                ASSERT(FALSE);
                break;
        }
    }
}

INT
IoCreateReadTask
    (
        IN TASK_PRIORITY priority, 
        IN EVENT event, 
        IN IO_READ_FUNC readFunc, 
        IN STRING name
    )
{
    INT result = Create(priority, IopReadTask);

    if(SUCCESSFUL(result))
    {
        INT taskId = result;
        IO_READ_TASK_PARAMS params;

        params.notifierParams.event = event;
        params.notifierParams.read = readFunc;
        params.name = name;

        result = Send(taskId, 
                      &params, 
                      sizeof(params), 
                      NULL, 
                      0);
    }

    return result;
}

INT
Read
    (
        IN IO_DEVICE* device, 
        IN PVOID buffer, 
        IN UINT bufferLength
    )
{
    IO_READ_REQUEST request = { ReadRequest, buffer, bufferLength };

    return Send(device->readTaskID, 
                &request, 
                sizeof(request), 
                NULL, 
                0);
}
