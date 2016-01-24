#include "task.h"
#include "arm.h"
#include "scheduler.h"
#include "stack.h"
#include "task_descriptor.h"

#include <rtosc/assert.h>

#define ERROR_PRIORITY_INVALID          -1
#define ERROR_OUT_OF_TASK_DESCRIPTORS   -2
#define ERROR_OUT_OF_STACK_SPACE        -3

VOID
TaskInit
    (
        VOID
    )
{
    RT_STATUS status;

    status = StackInit();

    ASSERT(RT_SUCCESS(status), "Failed to initialize stack manager. \r\n");

    TaskDescriptorInit();

    // Set up canaries

    // TODO: Make a TaskBootstrap function.  Ask Taylor for
    //       more details if you need.  This would require
    //       adding the start function to TASK_DESCRIPTOR
}

BOOLEAN
TaskIsPriorityValid
    (
        IN TASK_PRIORITY priority
    )
{
    return (0 <= priority) && (priority < NumPriority);
}

INT
TaskCreate
    (
        IN INT parentTaskId,
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc,
        OUT TASK_DESCRIPTOR** taskDescriptor
    )
{
    STACK stack;

    if (!TaskIsPriorityValid(priority))
    {
        return ERROR_PRIORITY_INVALID;
    }

    if (RT_FAILURE(StackGet(&stack)))
    {
        return ERROR_OUT_OF_STACK_SPACE;
    }

    if (RT_FAILURE(TaskDescriptorCreate(parentTaskId, priority, startFunc, stack, taskDescriptor)))
    {
        return ERROR_OUT_OF_TASK_DESCRIPTORS;
    }

    return (*taskDescriptor)->taskId;
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
    task->stackPointer = GetUserSP();
}
