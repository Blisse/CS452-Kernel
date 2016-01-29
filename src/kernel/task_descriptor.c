#include "task_descriptor.h"

#include "rtos.h"

#define TASK_INITIAL_CPSR   0x10

static INT g_taskDescriptorIds[NUM_TASK_DESCRIPTORS + 1];
static RT_CIRCULAR_BUFFER g_taskDescriptorIdQueue;

static TASK_DESCRIPTOR g_taskDescriptors[NUM_TASK_DESCRIPTORS];

static
inline
VOID
TaskDescriptorReset
    (
        IN TASK_DESCRIPTOR* td
    )
{
    td->taskId = -1;
    td->parentTaskId = -1;
    td->state = ZombieState;
    td->priority = IdlePriority;
    td->stackPointer = NULLPTR;
}

RT_STATUS
TaskDescriptorInit
    (
        VOID
    )
{
    UINT i;

    for(i = 0; i < NUM_TASK_DESCRIPTORS; i++)
    {
        TaskDescriptorReset(&g_taskDescriptors[i]);
    }

    RtCircularBufferInit(&g_taskDescriptorIdQueue, g_taskDescriptorIds, sizeof(g_taskDescriptorIds));

    int status = STATUS_SUCCESS;

    for(i = 0; i < NUM_TASK_DESCRIPTORS && RT_SUCCESS(status); i++)
    {
        status = RtCircularBufferAdd(&g_taskDescriptorIdQueue, &i, sizeof(i));
    }

    return status;
}

static
inline
VOID
TaskDescriptorpInitializeStack
    (
        IN TASK_DESCRIPTOR* td,
        IN STACK* stack,
        IN TASK_START_FUNC startFunc
    )
{
    UINT* stackPointer = ((UINT*) ptr_add(stack->top, stack->size)) - sizeof(UINT);

    *stackPointer = (UINT) Exit;
    *(stackPointer - 10) = (UINT) startFunc;
    *(stackPointer - 11) = TASK_INITIAL_CPSR;
    stackPointer -= 12;

    td->stack = *stack;
    td->stackPointer = stackPointer;
}

RT_STATUS
TaskDescriptorCreate
    (
        IN INT parentTaskId,
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc,
        IN STACK* stack,
        OUT TASK_DESCRIPTOR** td
    )
{
    RT_STATUS status;

    INT id;

    status = RtCircularBufferGetAndRemove(&g_taskDescriptorIdQueue, &id, sizeof(id));

    if (RT_SUCCESS(status))
    {
        TASK_DESCRIPTOR* newTd = TaskDescriptorIndex(id);

        newTd->taskId = id;
        newTd->parentTaskId = parentTaskId;
        newTd->state = ReadyState;
        newTd->priority = priority;

        TaskDescriptorpInitializeStack(newTd, stack, startFunc);

        RtCircularBufferInit(&newTd->messages, newTd->messagesBuffer, sizeof(newTd->messagesBuffer));

        *td = newTd;
    }

    return status;
}

RT_STATUS
TaskDescriptorDestroy
    (
        IN INT taskId
    )
{
    TASK_DESCRIPTOR* td;

    RT_STATUS status = TaskDescriptorGet(taskId, &td);

    if (RT_SUCCESS(status))
    {
        taskId += NUM_TASK_DESCRIPTORS;

        status = RtCircularBufferAdd(&g_taskDescriptorIdQueue, &taskId, sizeof(taskId));

        TaskDescriptorReset(td);
    }

    return status;
}

inline
TASK_DESCRIPTOR*
TaskDescriptorIndex
    (
        IN INT taskId
    )
{
    return &g_taskDescriptors[taskId % NUM_TASK_DESCRIPTORS];
}

inline
RT_STATUS
TaskDescriptorGet
    (
        IN INT taskId,
        OUT TASK_DESCRIPTOR** td
    )
{
    TASK_DESCRIPTOR* getTd = TaskDescriptorIndex(taskId);

    if (getTd->taskId == taskId)
    {
        *td = getTd;
        return STATUS_SUCCESS;
    }

    return STATUS_NOT_FOUND;
}

inline
RT_STATUS
TaskDescriptorMessagePush
    (
        IN TASK_DESCRIPTOR* td,
        IN INT senderId,
        IN PVOID message,
        IN INT messageLength
    )
{
    TASK_MESSAGE taskMessage;

    taskMessage.senderId = senderId;
    taskMessage.message = message;
    taskMessage.messageLength = messageLength;

    return RtCircularBufferAdd(&td->messages, &taskMessage, sizeof(taskMessage));
}

inline
RT_STATUS
TaskDescriptorMessagePop
    (
        IN TASK_DESCRIPTOR* td,
        OUT TASK_MESSAGE* message
    )
{
    return RtCircularBufferGetAndRemove(&td->messages, message, sizeof(*message));
}

inline
VOID
TaskDescriptorSetReturnValue
    (
        IN TASK_DESCRIPTOR* td,
        INT returnValue
    )
{
    *(td->stackPointer + 1) = returnValue;
}
