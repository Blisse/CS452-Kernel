#include "io.h"

#include <rtosc/assert.h>
#include "courier.h"

typedef struct _IO_WRITE_TASK_NOTIFIER_PARAMS
{
    EVENT event;
} IO_WRITE_TASK_NOTIFIER_PARAMS;

typedef struct _IO_WRITE_TASK_PARAMS
{
    IO_WRITE_TASK_NOTIFIER_PARAMS notifierParams;
    IO_WRITE_FUNC write;
    STRING name;
} IO_WRITE_TASK_PARAMS;

typedef enum _IO_WRITE_REQUEST_TYPE
{
    NotifierRequest = 0, 
    WriteRequest
} IO_WRITE_REQUEST_TYPE;

typedef struct _IO_WRITE_REQUEST
{
    IO_WRITE_REQUEST_TYPE type;
    PVOID buffer;
    UINT bufferLength;
} IO_WRITE_REQUEST;

static
VOID
IopWriteNotifierTask
    (
        VOID
    )
{
    IO_WRITE_REQUEST request = { NotifierRequest };
    IO_WRITE_TASK_NOTIFIER_PARAMS params;
    INT parentId;

    // Get parameters
    VERIFY(SUCCESSFUL(Receive(&parentId, &params, sizeof(params))));
    VERIFY(SUCCESSFUL(Reply(parentId, NULL, 0)));

    // Setup a courier
    CourierCreateTask(Priority30, MyTid(), parentId);

    // Run the notifier
    while(1)
    {
        // Wait for the event to come in
        VERIFY(SUCCESSFUL(AwaitEvent(params.event)));

        // Send it off to the write server
        CourierPickup(&request, sizeof(request));
    }
}

static
VOID
IopWriteTask
    (
        VOID
    )
{
    IO_WRITE_TASK_PARAMS params;
    INT sender;
    INT notifierTaskId;

    // Get parameters
    VERIFY(SUCCESSFUL(Receive(&sender, &params, sizeof(params))));
    VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));

    // Register with the name server
    VERIFY(SUCCESSFUL(RegisterAs(params.name)));

    // Set up the notifier task
    notifierTaskId = Create(HighestSystemPriority, IopWriteNotifierTask);
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
        IO_WRITE_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&sender, &request, sizeof(request))));

        switch(request.type)
        {
            case NotifierRequest:
                ASSERT(FALSE);
                break;

            case WriteRequest:
                ASSERT(FALSE);
                break;

            default:
                ASSERT(FALSE);
                break;
        }
    }
}

INT
IoCreateWriteTask
    (
        IN TASK_PRIORITY priority, 
        IN EVENT event, 
        IN IO_WRITE_FUNC writeFunc, 
        IN STRING name
    )
{
    INT result = Create(priority, IopWriteTask);

    if(SUCCESSFUL(result))
    {
        INT taskId = result;
        IO_WRITE_TASK_PARAMS params;

        params.notifierParams.event = event;
        params.write = writeFunc;
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
Write
    (
        IN IO_DEVICE* device, 
        IN PVOID buffer,
        IN UINT bufferLength
    )
{
    IO_WRITE_REQUEST request = { WriteRequest, buffer, bufferLength };

    return Send(device->writeTaskID, 
                &request, 
                sizeof(request), 
                NULL, 
                0);
}
