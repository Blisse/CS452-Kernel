#include "stack.h"

#include <rtosc/buffer.h>
#include "task.h"

#define CANARY 0x12341234

#define STACK_SIZE  0x10000
#define STACK_ADDRESS_START 0x00400000
#define STACK_ADDRESS_END   0x01F00000

static INT g_AvailableStacksBuffer[NUM_TASK_DESCRIPTORS + 1];
static RT_CIRCULAR_BUFFER g_AvailableStacksQueue;

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
StackInit
    (
        VOID
    )
{
    UINT i;
    RT_STATUS status = STATUS_SUCCESS;    

    RtCircularBufferInit(&g_AvailableStacksQueue, 
                         g_AvailableStacksBuffer, 
                         sizeof(g_AvailableStacksBuffer));
    
    for(i = 0; i < NUM_TASK_DESCRIPTORS && RT_SUCCESS(status); i++)
    {
        UINT* stackAddr = StackpCalculateAddress(i);

        if (stackAddr > (UINT*) STACK_ADDRESS_END)
        {
            return STATUS_STACK_SPACE_OVERFLOW;
        }

        *stackAddr = CANARY;

        status = RtCircularBufferAdd(&g_AvailableStacksQueue, &i, sizeof(i));
    }

    return status;
}

RT_STATUS
StackGet
    (
        STACK* stack
    )
{
    UINT stackId;
    RT_STATUS status = RtCircularBufferGetAndRemove(&g_AvailableStacksQueue, 
                                                    &stackId, 
                                                    sizeof(stackId));

    if (RT_SUCCESS(status))
    {
        stack->id = stackId;
        stack->top = StackpCalculateAddress(stackId);
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

inline
BOOLEAN
StackVerify
    (
        IN STACK* stack
    )
{
    return CANARY == *stack->top;
}
