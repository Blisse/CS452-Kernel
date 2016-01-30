#include "init.h"

#include <bwio/bwio.h>
#include <rtos.h>

#define HIGH_PRIORITY       1
#define MEDIUM_PRIORITY     2
#define LOW_PRIORITY        3

VOID
UserTask
    (
        VOID
    )
{
    INT myTid = MyTid();
    INT myParentTid = MyParentTid();

    bwprintf(BWCOM2, "%d %d\r\n", myTid, myParentTid);

    Pass();

    bwprintf(BWCOM2, "%d %d\r\n", myTid, myParentTid);
}

VOID
SendTask
    (
        VOID
    )
{
    INT i;

    for (i = 0; i < 3; i++)
    {
        INT reply;
        INT bytesReceived;

        bytesReceived = Send(2, &i, sizeof(i), &reply, sizeof(reply));

        bwprintf(BWCOM2, "Reply of length %d.  Contains %d \r\n", bytesReceived, reply);
    }
}

VOID
ReceiveTask
    (
        VOID
    )
{
    INT i;

    for (i = 4; i < 7; i++)
    {
        INT senderId;
        INT message;
        INT bytesReceived;

        bytesReceived = Receive(&senderId, &message, sizeof(message));

        bwprintf(BWCOM2, "Received %d bytes from %d containing %d \r\n", bytesReceived, senderId, message);

        Reply(senderId, &i, sizeof(i));
    }
}

VOID
InitTask
    (
        VOID
    )
{
    INT userTaskId;

    userTaskId = Create(LOW_PRIORITY, SendTask);
    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    userTaskId = Create(LOW_PRIORITY, ReceiveTask);
    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    bwprintf(BWCOM2, "FirstUserTask: exiting\r\n");
}
