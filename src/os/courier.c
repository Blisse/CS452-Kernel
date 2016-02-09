#include "courier.h"

#include <rtosc/assert.h>

#define COURIER_BUFFER_SIZE 64

typedef struct _COURIER_PARAMETERS
{
    INT sourceTask;
    INT destinationTask;
} COURIER_PARAMETERS;

static
VOID
CourierpTask
    (
        VOID
    )
{
    CHAR buffer[COURIER_BUFFER_SIZE];
    INT bytesReceived;
    COURIER_PARAMETERS parameters;
    INT sender;

    Receive(&sender, &parameters, sizeof(parameters));
    Reply(sender, NULL, 0);

    while(1)
    {   
        bytesReceived = Send(parameters.sourceTask,
                             NULL, 
                             0, 
                             buffer, 
                             sizeof(buffer));

        ASSERT(SUCCESSFUL(bytesReceived));

        bytesReceived = Send(parameters.destinationTask, 
                             buffer, 
                             bytesReceived, 
                             NULL, 
                             0);

        ASSERT(SUCCESSFUL(bytesReceived));
    }
}

VOID
CourierPickup
    (
        IN PVOID buffer, 
        IN INT size
    )
{
    INT courierId;

    VERIFY(SUCCESSFUL(Receive(&courierId, NULL, 0)));
    VERIFY(SUCCESSFUL(Reply(courierId, buffer, size)));
}

VOID
CourierCreateTask
    (
        IN TASK_PRIORITY priority, 
        IN INT sourceTask, 
        IN INT destinationTask
    )
{
    COURIER_PARAMETERS parameters = { sourceTask, destinationTask };
    INT courierTaskId = Create(priority, CourierpTask);

    ASSERT(SUCCESSFUL(courierTaskId));

    VERIFY(SUCCESSFUL(Send(courierTaskId, 
                      &parameters, 
                      sizeof(parameters), 
                      NULL, 
                      0)));
}
