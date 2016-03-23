#include "shutdown.h"

#include <rtosc/assert.h>
#include <rtkernel.h>
#include <rtos.h>
#include "idle.h"

#define SHUTDOWN_TASK_NAME "shutdown"

typedef enum _SHUTDOWN_REQUEST_TYPE
{
    RegisterRequest = 0,
    ShutdownRequest
} SHUTDOWN_REQUEST_TYPE;

typedef struct _SHUTDOWN_REQUEST
{
    SHUTDOWN_REQUEST_TYPE type;
    SHUTDOWN_HOOK_FUNC shutdownHook;
} SHUTDOWN_REQUEST;

static
VOID
ShutdownpTask()
{
    SHUTDOWN_HOOK_FUNC shutdownHooks[NUM_TASKS];
    for (UINT i = 0; i < NUM_TASKS; i++)
    {
        shutdownHooks[i] = NULL;
    }

    VERIFY(SUCCESSFUL(RegisterAs(SHUTDOWN_TASK_NAME)));

    BOOLEAN running = TRUE;
    while (running)
    {
        INT senderId;
        SHUTDOWN_REQUEST request;

        VERIFY(SUCCESSFUL(Receive(&senderId, &request, sizeof(request))));

        switch(request.type)
        {
            case RegisterRequest:
                shutdownHooks[senderId % NUM_TASKS] = request.shutdownHook;
                break;

            case ShutdownRequest:
                running = FALSE;
                break;

            default:
                ASSERT(FALSE);
                break;
        }

        VERIFY(SUCCESSFUL(Reply(senderId, NULL, 0)));
    }

    // Run shutdown hooks
    for (UINT i = 0; i < NUM_TASKS; i++)
    {
        SHUTDOWN_HOOK_FUNC shutdownHook = shutdownHooks[i];

        if (shutdownHook != NULL)
        {
            shutdownHook();
        }
    }

    // Give the system some time to perform shutdown
    Delay(100);

    // Kill the idle task
    IdleQuit();
}

static
inline
INT
ShutdownpSendRequest
    (
        IN SHUTDOWN_REQUEST* request
    )
{
    INT result = WhoIs(SHUTDOWN_TASK_NAME);

    if (SUCCESSFUL(result))
    {
        INT shutdownTaskId = result;

        result = Send(shutdownTaskId,
                      request,
                      sizeof(*request),
                      NULL,
                      0);
    }

    return result;
}

INT
ShutdownRegisterHook
    (
        IN SHUTDOWN_HOOK_FUNC shutdownFunc
    )
{
    SHUTDOWN_REQUEST request = { RegisterRequest, shutdownFunc };

    return ShutdownpSendRequest(&request);
}

VOID
Shutdown()
{
    SHUTDOWN_REQUEST request = { ShutdownRequest };

    VERIFY(SUCCESSFUL(ShutdownpSendRequest(&request)));
}

VOID
ShutdownCreateTask()
{
    VERIFY(SUCCESSFUL(Create(LowestSystemPriority, ShutdownpTask)));
}
