#include "init.h"

#include "idle.h"
#include "clock_server.h"
#include "name_server.h"
#include <rtosc/assert.h>
#include <bwio/bwio.h>

typedef struct _CLOCK_CLIENT_DELAY_REQUEST
{
    INT delay;
    INT repeat;
} CLOCK_CLIENT_DELAY_REQUEST;

static
VOID
UserClientTask
    (
        VOID
    )
{
    INT taskId = MyTid();
    INT parentTaskId = MyParentTid();

    CLOCK_CLIENT_DELAY_REQUEST request;
    Send(parentTaskId, NULL, 0, &request, sizeof(request));

    INT i;
    for (i = 0; i < request.repeat; i++)
    {
        Delay(request.delay);
        bwprintf(BWCOM2, "%d %d %d/%d\r\n", taskId, request.delay, i+1, request.repeat);
    }
}

static
VOID
UserTask
    (
        VOID
    )
{
    INT priority3TaskId = Create(Priority20, UserClientTask);
    INT priority4TaskId = Create(Priority19, UserClientTask);
    INT priority5TaskId = Create(Priority18, UserClientTask);
    INT priority6TaskId = Create(Priority17, UserClientTask);

    INT i;
    for (i = 0; i < 4; i++)
    {
        INT taskId;
        Receive(&taskId, NULL, 0);
    }

    CLOCK_CLIENT_DELAY_REQUEST priority3Request = { 10, 20 };
    Reply(priority3TaskId, &priority3Request, sizeof(priority3Request));

    CLOCK_CLIENT_DELAY_REQUEST priority4Request = { 23, 9 };
    Reply(priority4TaskId, &priority4Request, sizeof(priority4Request));

    CLOCK_CLIENT_DELAY_REQUEST priority5Request = { 33, 6 };
    Reply(priority5TaskId, &priority5Request, sizeof(priority5Request));

    CLOCK_CLIENT_DELAY_REQUEST priority6Request = { 71, 3 };
    Reply(priority6TaskId, &priority6Request, sizeof(priority6Request));
}

VOID
InitTask
    (
        VOID
    )
{
    IdleCreateTask();

    NameServerCreateTask();

    ClockServerCreateTask();

    UserTask();
}
