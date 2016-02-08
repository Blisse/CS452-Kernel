#include "priority_queue.h"

VOID
RtPriorityQueueInit
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        IN PVOID buffers,
        IN RT_CIRCULAR_BUFFER* queues,
        IN UINT typeSize,
        IN UINT priorities,
        IN UINT capacity
    )
{

    priorityQueue->buffers = buffers;
    priorityQueue->queues = queues;
    priorityQueue->bitmask = 0;

    UINT actualBufferCapacity = typeSize * capacity;

    UINT i;
    for(i = 0; i < priorities; i++)
    {
        RtCircularBufferInit(&queues[i], buffers + (i * actualBufferCapacity), actualBufferCapacity);
    }
}

static
inline
BOOLEAN
IsPower2
    (
        IN UINT x
    )
{
    // This code is courtesy of:
    // http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    return (x && !(x & (x - 1)));
}

static
inline
UINT
Log2
    (
        IN UINT v
    )
{
    // This code is taken from:
    // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
    static const INT MultiplyDeBruijnBitPosition[32] =
    {
      0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
      8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };

    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    return MultiplyDeBruijnBitPosition[(UINT)(v * 0x07C4ACDDU) >> 27];
}

static
inline
UINT
Power2Log2
    (
        IN UINT v
    )
{
    // This code is taken from:
    // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
    static const INT MultiplyDeBruijnBitPosition[32] =
    {
      0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
      31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };

    return MultiplyDeBruijnBitPosition[(UINT)(v * 0x077CB531U) >> 27];
}

inline
RT_STATUS
RtPriorityQueuePush
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        IN UINT priority,
        IN PVOID sourceBuffer,
        IN UINT bytesToAdd
    )
{
    if(likely(IsPower2(priority)))
    {
        UINT index = Power2Log2(priority);
        RT_CIRCULAR_BUFFER* queue = &priorityQueue->queues[index];
        RT_STATUS status = RtCircularBufferPush(queue, sourceBuffer, bytesToAdd);
        
        if (RT_SUCCESS(status))
        {
            priorityQueue->bitmask |= priority;
        }

        return status;
    }
    else
    {
        return STATUS_INVALID_PARAMETER;
    }
}

RT_STATUS
RtPriorityQueuePeek
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        OUT PVOID targetBuffer,
        IN UINT bytesToGet
    )
{
    if(likely(priorityQueue->bitmask))
    {
        UINT index = Log2(priorityQueue->bitmask);
        RT_CIRCULAR_BUFFER* queue = &priorityQueue->queues[index];

        return RtCircularBufferPeek(queue, targetBuffer, bytesToGet);
    }
    else
    {
        return STATUS_NOT_FOUND;
    }
}

inline
RT_STATUS
RtPriorityQueuePop
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        IN UINT bytesToRemove
    )
{
    if(likely(priorityQueue->bitmask))
    {
        UINT index = Log2(priorityQueue->bitmask);
        RT_CIRCULAR_BUFFER* queue = &priorityQueue->queues[index];
        RT_STATUS status = RtCircularBufferPop(queue, bytesToRemove);

        if(RT_SUCCESS(status) && RtCircularBufferIsEmpty(queue))
        {
            priorityQueue->bitmask &= ~(1 << index);
        }

        return status;
    }
    else
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
}

inline
RT_STATUS
RtPriorityQueuePeekAndPop
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        OUT PVOID targetBuffer,
        IN UINT bytesToGet
    )
{
    RT_STATUS status = RtPriorityQueuePeek(priorityQueue, targetBuffer, bytesToGet);

    if(RT_SUCCESS(status))
    {
        status = RtPriorityQueuePop(priorityQueue, bytesToGet);
    }

    return status;
}
