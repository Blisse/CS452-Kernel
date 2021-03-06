#include "task.h"

#include <rtosc/assert.h>
#include <rtosc/string.h>

#include "ipc.h"
#include "scheduler.h"
#include "stack.h"

#define CANARY 0x12341234
#define TASK_INITIAL_CPSR 0x10
#define RETURN_VALUE_OFFSET 8

VOID
TaskInit()
{
    VERIFY(RT_SUCCESS(StackInit()));
    VERIFY(RT_SUCCESS(TaskDescriptorInit()));
}

static
inline
BOOLEAN
TaskpIsPriorityValid
    (
        IN TASK_PRIORITY priority
    )
{
    // Check to see if priority is a power of 2
    // This code is courtesy of:
    // http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    return priority && !(priority & (priority - 1));
}

static
inline
UINT*
TaskpSetupStack
    (
        IN STACK* stack,
        IN TASK_START_FUNC startFunc
    )
{
    UINT* stackPointer = ((UINT*) ptr_add(stack->top, stack->size)) - sizeof(UINT);

    *stackPointer = (UINT) Exit;
    *(stackPointer - 14) = TASK_INITIAL_CPSR;
    *(stackPointer - 15) = (UINT) startFunc;
    stackPointer -= 15;

    return stackPointer;
}

RT_STATUS
TaskCreate
    (
        IN TASK_DESCRIPTOR* parent,
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc,
        OUT TASK_DESCRIPTOR** td
    )
{
    RT_STATUS status;

    if (TaskpIsPriorityValid(priority))
    {
        TASK_DESCRIPTOR* newTd;

        status = TaskDescriptorAllocate(&newTd);

        if (RT_SUCCESS(status))
        {
            status = StackAllocate(&newTd->stack);

            if (RT_SUCCESS(status))
            {
                newTd->parentTaskId = NULL == parent ? 0 : parent->taskId;
                newTd->state = ReadyState;
                newTd->priority = priority;
                newTd->stackPointer = TaskpSetupStack(newTd->stack, startFunc);
                *(newTd->stack->top) = CANARY;
                IpcInitializeMailbox(newTd);

                status = SchedulerAddTask(newTd);

                if (RT_SUCCESS(status))
                {
                    *td = newTd;
                }
            }
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}

VOID
TaskDestroy
    (
        IN TASK_DESCRIPTOR* td
    )
{
    td->state = ZombieState;
    IpcDrainMailbox(td);
    VERIFY(RT_SUCCESS(StackDeallocate(td->stack)));
    VERIFY(RT_SUCCESS(TaskDescriptorDeallocate(td)));
}

BOOLEAN
TaskValidate
    (
        IN TASK_DESCRIPTOR* task
    )
{
    return CANARY == *(task->stack->top);
}

VOID
TaskSetReturnValue
    (
        IN TASK_DESCRIPTOR* td,
        IN INT returnValue
    )
{
    *(UINT*)(ptr_add(td->stackPointer, RETURN_VALUE_OFFSET)) = returnValue;
}

VOID
TaskStoreAsyncParameter
    (
        IN TASK_DESCRIPTOR* td,
        IN PVOID parameter,
        IN UINT size
    )
{
    RtMemcpy(ptr_add(td->stackPointer, -1 * (size + sizeof(UINT))),
             parameter,
             size);
}

VOID
TaskRetrieveAsyncParameter
    (
        IN TASK_DESCRIPTOR* td,
        IN PVOID parameter,
        IN UINT size
    )
{
    RtMemcpy(parameter,
             ptr_add(td->stackPointer, -1 * (size + sizeof(UINT))),
             size);
}
