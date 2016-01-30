#include "task.h"

#include "arm.h"
#include "ipc.h"
#include <rtosc/assert.h>
#include "rtos.h"
#include "scheduler.h"
#include "stack.h"

#define CANARY 0x12341234
#define TASK_INITIAL_CPSR   0x10

VOID
TaskInit
    (
        VOID
    )
{
    RT_STATUS status = StackInit();

    ASSERT(RT_SUCCESS(status), "Failed to initialize stack manager. \r\n");

    status = TaskDescriptorInit();

    ASSERT(RT_SUCCESS(status), "Failed to initialize task descriptors. \r\n");
}

static
inline
BOOLEAN
TaskpIsPriorityValid
    (
        IN TASK_PRIORITY priority
    )
{
    return (SystemPriority <= priority) && (priority < NumPriority);
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
    *(stackPointer - 10) = (UINT) startFunc;
    *(stackPointer - 11) = TASK_INITIAL_CPSR;
    stackPointer -= 12;

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

        if(RT_SUCCESS(status))
        {
            status = StackAllocate(&newTd->stack);

            if(RT_SUCCESS(status))
            {
                newTd->parentTaskId = NULL == parent ? 0 : parent->taskId;
                newTd->state = ReadyState;
                newTd->priority = priority;
                newTd->stackPointer = TaskpSetupStack(newTd->stack, startFunc);
                *(newTd->stack->top) = CANARY;
                IpcInitializeMailbox(newTd);

                status = SchedulerAddTask(newTd);

                if(RT_SUCCESS(status))
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
    VERIFY(RT_SUCCESS(StackDeallocate(td->stack)), "Failed to destroy task's stack \r\n");
    VERIFY(RT_SUCCESS(TaskDescriptorDeallocate(td)), "Failed to destroy task descriptor \r\n");
}

inline
BOOLEAN
TaskValidate
    (
        IN TASK_DESCRIPTOR* task
    )
{
    return CANARY == *(task->stack->top);
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

inline
VOID
TaskSetReturnValue
    (
        IN TASK_DESCRIPTOR* td, 
        IN INT returnValue
    )
{
    *(td->stackPointer) = returnValue;
}
