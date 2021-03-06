#include "stack.h"

#include <rtosc/buffer.h>
#include <rtkernel.h>

#define STACK_SIZE  0x80000
#define STACK_ADDRESS_START 0x00400000
#define STACK_ADDRESS_END   0xFFF00000

static STACK g_stacks[NUM_TASKS];
static STACK* g_availableStacksBuffer[NUM_TASKS];
static RT_CIRCULAR_BUFFER g_availableStacksQueue;

static
UINT*
StackpCalculateAddress
    (
        IN UINT stackId
    )
{
    return (UINT*) (STACK_ADDRESS_START + (STACK_SIZE * stackId));
}

RT_STATUS
StackInit()
{
    UINT i;
    RT_STATUS status = STATUS_SUCCESS;

    RtCircularBufferInit(&g_availableStacksQueue,
                         g_availableStacksBuffer,
                         sizeof(g_availableStacksBuffer));

    for (i = 0; i < NUM_TASKS && RT_SUCCESS(status); i++)
    {
        STACK* stack = &g_stacks[i];
        stack->top = StackpCalculateAddress(i);
        stack->size = STACK_SIZE;

        if (stack->top > (UINT*) STACK_ADDRESS_END)
        {
            return STATUS_STACK_SPACE_OVERFLOW;
        }

        status = StackDeallocate(stack);
    }

    return status;
}

RT_STATUS
StackAllocate
    (
        OUT STACK** stack
    )
{
    return RtCircularBufferPeekAndPop(&g_availableStacksQueue,
                                      stack,
                                      sizeof(*stack));
}

RT_STATUS
StackDeallocate
    (
        IN STACK* stack
    )
{
    return RtCircularBufferPush(&g_availableStacksQueue, &stack, sizeof(stack));
}
