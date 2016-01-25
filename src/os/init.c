#include "init.h"

#include "rtos.h"
#include <bwio/bwio.h>

VOID
UserTask
    (
        VOID
    )
{
    bwprintf(BWCOM2, "UserTask: task id %d\r\n", MyTid());

    Pass();

    bwprintf(BWCOM2, "UserTask: task id %d\r\n", MyTid());

    Exit();
}

VOID
InitTask
    (
        VOID
    )
{
    bwprintf(BWCOM2, "FirstUserTask: starting\r\n");

    int priority = MyTid();

    bwprintf(BWCOM2, "FirstUserTask: task id %d\r\n", priority);

    int userTaskId;

    userTaskId = Create(priority-1, UserTask);

    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    userTaskId = Create(priority-1, UserTask);

    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    userTaskId = Create(priority+1, UserTask);

    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    userTaskId = Create(priority+1, UserTask);

    bwprintf(BWCOM2, "Created: %d\r\n", userTaskId);

    bwprintf(BWCOM2, "FirstUserTask: exiting\r\n");

    Exit();
}
