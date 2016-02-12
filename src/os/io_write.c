#include "io.h"

#include <rtosc/assert.h>
#include <rtosc/buffer.h>

#define DEFAULT_BUFFER_SIZE 1024

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

    // Run the notifier
    while(1)
    {
        // Wait for the event to come in
        VERIFY(SUCCESSFUL(AwaitEvent(params.event)));

        // Send it off to the write server
        VERIFY(SUCCESSFUL(Send(parentId, &request, sizeof(request), NULL, 0)));
    }
}

static
inline
VOID
IopPerformWrite
    (
        IN INT notifierTaskId,
        IN IO_WRITE_FUNC write, 
        IN RT_CIRCULAR_BUFFER* buffer
    )
{
    CHAR c;

    // Grab the next character to be written
    VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(buffer, &c, sizeof(c))));

    // Write the character
    write(c);

    // Unblock the notifier
    VERIFY(SUCCESSFUL(Reply(notifierTaskId, NULL, 0)));
}

static
VOID
IopWriteTask
    (
        VOID
    )
{
    CHAR underlyingTransmitBuffer[DEFAULT_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER transmitBuffer;
    BOOLEAN canWrite;
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

    // Initialize task variables
    canWrite = FALSE;
    RtCircularBufferInit(&transmitBuffer, 
                         underlyingTransmitBuffer, 
                         sizeof(underlyingTransmitBuffer));

    // Run the server
    while(1)
    {
        IO_WRITE_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&sender, &request, sizeof(request))));

        switch(request.type)
        {
            case NotifierRequest:
                if(RtCircularBufferIsEmpty(&transmitBuffer))
                {
                    canWrite = TRUE;
                }
                else
                {
                    IopPerformWrite(notifierTaskId, params.write, &transmitBuffer);
                }
                
                break;

            case WriteRequest:
                VERIFY(RT_SUCCESS(RtCircularBufferPush(&transmitBuffer, 
                                                       request.buffer, 
                                                       request.bufferLength)));

                if(canWrite)
                {
                    canWrite = FALSE;
                    IopPerformWrite(notifierTaskId, params.write, &transmitBuffer);
                }

                VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));
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
