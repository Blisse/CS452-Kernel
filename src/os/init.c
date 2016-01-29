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
    INT myTid = MyTid();
    INT myParentTid = MyParentTid();

    bwprintf(BWCOM2, "Send Task %d %d\r\n", myTid, myParentTid);

    INT i;
    for (i = 0; i < 3; i++)
    {
        bwprintf(BWCOM2, "%d Send Begin \r\n", i);

        int reply;
        Send(3, &i, sizeof(i), &reply, sizeof(reply));

        bwprintf(BWCOM2, "%d Send End\r\n", i);
    }
}

VOID
ReceiveTask
    (
        VOID
    )
{
    INT myTid = MyTid();
    INT myParentTid = MyParentTid();

    bwprintf(BWCOM2, "Receive Task %d %d\r\n", myTid, myParentTid);

    INT i;
    for (i = 0; i < 3; i++)
    {
        int senderId;
        int message;
        bwprintf(BWCOM2, "%d Receive & Reply Start \r\n", i);

        Receive(&senderId, &message, sizeof(message));

        bwprintf(BWCOM2, "%d Received Message %d \r\n", i, message);

        Reply(senderId, &i, sizeof(i));

        bwprintf(BWCOM2, "%d Receive & Reply End \r\n", i);
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
