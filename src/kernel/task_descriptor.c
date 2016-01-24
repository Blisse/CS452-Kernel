#include "task_descriptor.h"

#include <bwio/bwio.h>

static UINT NextFreeTaskId;
static TASK_DESCRIPTOR TaskDescriptors[NUM_TASK_DESCRIPTORS];

#define TASK_INITIAL_CSPR   0x10
#define TASK_INITIAL_RETURN 0x0

VOID
TaskDescriptorInit
    (
        VOID
    )
{
    NextFreeTaskId = 1;

    UINT i;
    for(i = 0; i < NUM_TASK_DESCRIPTORS; i++) {
        TaskDescriptorReset(&TaskDescriptors[i]);
    }
}

VOID
TaskDescriptorReset
    (
        IN TASK_DESCRIPTOR* td
    )
{
    td->taskId = -1;
    td->parentTaskId = -1;
    td->state = Zombie;
    td->priority = IdlePriority;
    td->startFunc = NULLPTR;
    td->stackPointer = NULLPTR;
}

static
inline
VOID
TaskDescriptorSetStack
    (
        IN TASK_DESCRIPTOR* td,
        IN STACK stack,
        IN TASK_START_FUNC startFunc
    )
{
    UINT* stackPointer = (UINT*) (stack.top + stack.size - 1);

    *(stackPointer - 10) = (UINT) startFunc;
    *(stackPointer - 11) = TASK_INITIAL_CSPR;
    *(stackPointer - 12) = TASK_INITIAL_RETURN;
    stackPointer -= 12;

    td->stack = stack;
    td->stackPointer = stackPointer;
}

static
inline
BOOLEAN
TaskDescriptorIsZombie
    (
        IN TASK_DESCRIPTOR* td
    )
{
    return (td->state == Zombie);
}

static
RT_STATUS
TaskDescriptorGetZombie
    (
        OUT TASK_DESCRIPTOR** td
    )
{
    UINT i;
    UINT idx = NextFreeTaskId % NUM_TASK_DESCRIPTORS;

    for(i = 0; i < NUM_TASK_DESCRIPTORS; i++)
    {
        if (TaskDescriptorIsZombie(&TaskDescriptors[idx]))
        {
            *td = &TaskDescriptors[idx];
            return STATUS_SUCCESS;
        }
        idx = (idx + 1) % NUM_TASK_DESCRIPTORS;
    }

    return STATUS_FAILURE;
}

RT_STATUS
TaskDescriptorCreate
    (
        IN INT parentTaskId,
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc,
        IN STACK stack,
        OUT TASK_DESCRIPTOR** td
    )
{
    TASK_DESCRIPTOR* zombieTd;

    if (RT_SUCCESS(TaskDescriptorGetZombie(&zombieTd)))
    {
        zombieTd->taskId = NextFreeTaskId;
        zombieTd->parentTaskId = parentTaskId;
        zombieTd->state = Ready;
        zombieTd->priority = priority;
        zombieTd->startFunc = startFunc;

        TaskDescriptorSetStack(zombieTd, stack, startFunc);

        NextFreeTaskId += 1;

        *td = zombieTd;

        return STATUS_SUCCESS;
    }

    return STATUS_FAILURE;
}

RT_STATUS
TaskDescriptorDestroy
    (
        IN INT taskId
    )
{
    TASK_DESCRIPTOR* td;

    if (RT_SUCCESS(TaskDescriptorGet(taskId, &td)))
    {
        TaskDescriptorReset(td);

        return STATUS_SUCCESS;
    }

    return STATUS_FAILURE;
}

RT_STATUS
TaskDescriptorGet
    (
        IN INT taskId,
        OUT TASK_DESCRIPTOR** td
    )
{
    UINT i;
    UINT idx = taskId % NUM_TASK_DESCRIPTORS;

    for(i = 0; i < NUM_TASK_DESCRIPTORS; i++)
    {
        if (TaskDescriptors[idx].taskId == taskId)
        {
            *td = &TaskDescriptors[idx];
            return STATUS_SUCCESS;
        }
        idx = (idx + 1) % NUM_TASK_DESCRIPTORS;
    }

    return STATUS_FAILURE;
}
