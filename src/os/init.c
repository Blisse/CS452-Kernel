#include "init.h"

#include "rtos.h"
#include <bwio/bwio.h>

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
}
