#include "stack.h"

#include <bwio/bwio.h>
#include <rtosc/buffer.h>
#include "task.h"

#define STACK_SIZE  0x10000
#define STACK_ADDRESS_START 0x00400000
#define STACK_ADDRESS_END   0x01F00000

static INT g_AvailableStacksBuffer[NUM_TASK_DESCRIPTORS + 1];
static RT_CIRCULAR_BUFFER g_AvailableStacksQueue;

RT_STATUS
StackInit
    (
        VOID
    )
{
    RtCircularBufferInit(&g_AvailableStacksQueue, g_AvailableStacksBuffer, sizeof(g_AvailableStacksBuffer));

    RT_STATUS status;
    UINT i;
    for(i = 0; i < NUM_TASK_DESCRIPTORS; i++)
    {
        if ((STACK_ADDRESS_START + (STACK_SIZE * i)) > STACK_ADDRESS_END)
        {
            return STATUS_STACK_SPACE_OVERFLOW;
        }

        status = RtCircularBufferAdd(&g_AvailableStacksQueue, &i, sizeof(i));

        if (RT_FAILURE(status))
        {
            return status;
        }
    }

    return status;
}

RT_STATUS
StackGet
    (
        STACK* stack
    )
{
    RT_STATUS status;
    INT stackId;

    status = RtCircularBufferGetAndRemove(&g_AvailableStacksQueue, &stackId, sizeof(stackId));

    if (RT_SUCCESS(status))
    {
        stack->id = stackId;
        stack->top = (UINT*) (STACK_ADDRESS_START + (STACK_SIZE * stackId));
        stack->size = STACK_SIZE;
    }

    return status;
}

inline
RT_STATUS
StackReturn
    (
        STACK* stack
    )
{
    return RtCircularBufferAdd(&g_AvailableStacksQueue, &stack->id, sizeof(stack->id));
}
