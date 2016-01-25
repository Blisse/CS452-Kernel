#include "init.h"

#include "rtos.h"
#include <bwio/bwio.h>

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

    Exit();
}

VOID
InitTask
    (
        VOID
    )
{
    INT userTaskId;

    userTaskId = Create(LOW_PRIORITY, UserTask);
    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    userTaskId = Create(LOW_PRIORITY, UserTask);
    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    userTaskId = Create(HIGH_PRIORITY, UserTask);
    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    userTaskId = Create(HIGH_PRIORITY, UserTask);
    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    bwprintf(BWCOM2, "FirstUserTask: exiting\r\n");

    Exit();
}
