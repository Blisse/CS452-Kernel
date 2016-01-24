#include "stack.h"

#include <bwio/bwio.h>
#include <rtosc/buffer.h>
#include "task.h"

#define STACK_ADDRESS_START 0x00400000
#define STACK_ADDRESS_END   0x01F00000

static INT AvailableStacksBuffer[NUMBER_OF_TASKS + 1];
static RT_CIRCULAR_BUFFER AvailableStacksQueue;

RT_STATUS
StackInit
    (
        VOID
    )
{
    RtCircularBufferInit(&AvailableStacksQueue, AvailableStacksBuffer, sizeof(AvailableStacksBuffer));

    UINT i;
    for(i = 0; i < NUMBER_OF_TASKS; i++)
    {
        if ((STACK_ADDRESS_START + (SIZE_OF_STACK * i)) > STACK_ADDRESS_END)
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
        stack->top = (UINT*) (STACK_ADDRESS_START + (SIZE_OF_STACK * stackId));
        stack->size = SIZE_OF_STACK;

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
