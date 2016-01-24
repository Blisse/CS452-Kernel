#include "stack.h"

#include <bwio/bwio.h>
#include <rtosc/buffer.h>
#include "task.h"

#define STACK_SIZE  0xFFFF
#define STACK_ADDRESS_START 0x00400000
#define STACK_ADDRESS_END   0x01F00000

static INT AvailableStacksBuffer[NUM_TASK_DESCRIPTORS + 1];
static RT_CIRCULAR_BUFFER AvailableStacksQueue;

RT_STATUS
StackInit
    (
        VOID
    )
{
    RtCircularBufferInit(&AvailableStacksQueue, AvailableStacksBuffer, sizeof(AvailableStacksBuffer));

    UINT i;
    for(i = 0; i < NUM_TASK_DESCRIPTORS; i++)
    {
        if ((STACK_ADDRESS_START + (STACK_SIZE * i)) > STACK_ADDRESS_END)
        {
            return STATUS_FAILURE;
        }

        if (RT_FAILURE(RtCircularBufferAdd(&AvailableStacksQueue, &i, sizeof(i))))
        {
            return STATUS_FAILURE;
        }
    }

    return STATUS_SUCCESS;
}

RT_STATUS
StackGet
    (
        STACK* stack
    )
{
    INT stackId;
    if (RT_SUCCESS(RtCircularBufferGetAndRemove(&AvailableStacksQueue, &stackId, sizeof(stackId))))
    {
        stack->id = stackId;
        stack->top = (UINT*) (STACK_ADDRESS_START + (STACK_SIZE * stackId));
        stack->size = STACK_SIZE;

        return STATUS_SUCCESS;
    }

    return STATUS_FAILURE;
}

RT_STATUS
StackReturn
    (
        STACK stack
    )
{
    return RtCircularBufferAdd(&AvailableStacksQueue, &stack.id, sizeof(stack.id));
}
