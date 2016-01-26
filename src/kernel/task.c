#include "task.h"
#include "arm.h"
#include "stack.h"

#include <rtosc/assert.h>

#define ERROR_PRIORITY_INVALID          -1
#define ERROR_OUT_OF_SPACE              -2

static
inline
BOOLEAN
TaskIsPriorityValid
    (
        IN TASK_PRIORITY priority
    )
{
    return (SystemPriority <= priority) && (priority < NumPriority);
}

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
        return ERROR_OUT_OF_SPACE;
    }

    if (RT_FAILURE(TaskDescriptorCreate(parentTaskId, priority, startFunc, &stack, taskDescriptor)))
    {
        return ERROR_OUT_OF_SPACE;
    }

    return (*taskDescriptor)->taskId;
}

inline
BOOLEAN
TaskValidate
    (
        IN TASK_DESCRIPTOR* task
    )
{
    return StackVerify(&task->stack);
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
