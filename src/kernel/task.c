#include "task.h"
#include "arm.h"
#include "scheduler.h"

VOID
TaskInit
    (
        VOID
    )
{
    // Set up stack pointers
    // Set up canaries

    // TODO: Make a TaskBootstrap function.  Ask Taylor for
    //       more details if you need.  This would require
    //       adding the start function to TASK_DESCRIPTOR

    // Call SchedulerAddTask
}

INT
TaskCreate
    (
        IN INT priority, 
        IN TASK_START_FUNC start
    )
{
    // Get a free stack
    // Get a free task id

    // Setup stack as per main.c!TestCreate

    // TODO: Remember to return specific error codes
    return -1;
}

BOOLEAN
TaskValidate
    (
        IN TASK_DESCRIPTOR* task
    )
{
    // TODO: Check canary
    return TRUE;
}

inline
VOID
TaskUpdate
    (
        IN TASK_DESCRIPTOR* task
    )
{
    task->stack = GetUserSP();
}
