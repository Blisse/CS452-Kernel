#include "io.h"

#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rtos.h>
#include "courier.h"

#define DEFAULT_BUFFER_SIZE 512

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
    ReadRequest,
    FlushRequest,
} IO_READ_REQUEST_TYPE;

typedef struct _IO_READ_REQUEST
{
    IO_READ_REQUEST_TYPE type;

    union
    {
        CHAR c;
        struct
        {
            PVOID buffer;
            UINT bufferLength;
        };
    };
} IO_READ_REQUEST;

typedef struct _IO_PENDING_READ
{
    UINT taskId;
    PVOID buffer;
    UINT bufferLength;
} IO_PENDING_READ;

static
VOID
IopReadNotifierTask()
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
    while (1)
    {
        CHAR c;

        // Wait for the read event to come in
        VERIFY(SUCCESSFUL(AwaitEvent(params.event)));

        // Read the new character
        c = params.read();

        // Send it off to the read server
        request.c = c;
        CourierPickup(&request, sizeof(request));
    }
}

static
VOID
IopReadTask()
{
    CHAR underlyingReceiveBuffer[DEFAULT_BUFFER_SIZE];
    RT_CIRCULAR_BUFFER receiveBuffer;
    IO_PENDING_READ underlyingPendingReadBuffer[NUM_TASKS];
    RT_CIRCULAR_BUFFER pendingReadQueue;
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

    // Initialize task parameters
    RtCircularBufferInit(&receiveBuffer,
                         underlyingReceiveBuffer,
                         sizeof(underlyingReceiveBuffer));
    RtCircularBufferInit(&pendingReadQueue,
                         underlyingPendingReadBuffer,
                         sizeof(underlyingPendingReadBuffer));

    // Run the server
    while (1)
    {
        IO_READ_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&sender, &request, sizeof(request))));

        switch(request.type)
        {
            case NotifierRequest:
                // Reply to the notifier
                VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));

                // Add the received character to the buffer
                VERIFY(RT_SUCCESS(RtCircularBufferPush(&receiveBuffer,
                                                       &request.c,
                                                       sizeof(request.c))));

                // Check to see if anyone is waiting on data
                if (!RtCircularBufferIsEmpty(&pendingReadQueue))
                {
                    IO_PENDING_READ pendingRead;

                    // Grab the task that is waiting on data
                    VERIFY(RT_SUCCESS(RtCircularBufferPeek(&pendingReadQueue,
                                                           &pendingRead,
                                                           sizeof(pendingRead))));

                    // Check to see if there is enough data to satisfy the task
                    if (RtCircularBufferSize(&receiveBuffer) >= pendingRead.bufferLength)
                    {
                        // Move data from the receive buffer to the task's buffer
                        VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&receiveBuffer,
                                                                     pendingRead.buffer,
                                                                     pendingRead.bufferLength)));

                        // Remove the task from the queue
                        VERIFY(RT_SUCCESS(RtCircularBufferPop(&pendingReadQueue,
                                                              sizeof(pendingRead))));

                        // Unblock the task
                        VERIFY(SUCCESSFUL(Reply(pendingRead.taskId, NULL, 0)));
                    }
                }

                break;

            case ReadRequest:
                // Check to see if there is enough data to satisfy the task
                if (RtCircularBufferSize(&receiveBuffer) >= request.bufferLength)
                {
                    // Move data from the receive buffer to the task's buffer
                    VERIFY(RT_SUCCESS(RtCircularBufferPeekAndPop(&receiveBuffer,
                                                                 request.buffer,
                                                                 request.bufferLength)));

                    // Unblock the task
                    VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));

                }
                else
                {
                    IO_PENDING_READ pendingRead = { sender, request.buffer, request.bufferLength };

                    // Add the task to a queue
                    VERIFY(RT_SUCCESS(RtCircularBufferPush(&pendingReadQueue,
                                                           &pendingRead,
                                                           sizeof(pendingRead))));
                }

                break;

            case FlushRequest:
            {
                INT receiveBufferSize = RtCircularBufferSize(&receiveBuffer);

                VERIFY(RT_SUCCESS(RtCircularBufferPop(&receiveBuffer, receiveBufferSize)));

                VERIFY(SUCCESSFUL(Reply(sender, NULL, 0)));

                break;
            }

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

    if (SUCCESSFUL(result))
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
    IO_READ_REQUEST request;

    request.type = ReadRequest;
    request.buffer = buffer;
    request.bufferLength = bufferLength;

    return Send(device->readTaskId,
                &request,
                sizeof(request),
                NULL,
                0);
}

INT
FlushInput
    (
        IN IO_DEVICE* device
    )
{
    IO_READ_REQUEST request;

    request.type = FlushRequest;

    return Send(device->readTaskId, &request, sizeof(request), NULL, 0);
}
