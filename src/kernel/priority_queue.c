#include "priority_queue.h"

VOID
RtPriorityQueueInit
    (
        IN RT_PRIORITY_QUEUE* priorityQueue
    )
{
    UINT i;

    for(i = 0; i < NUM_PRIORITY; i++)
    {
        RtCircularBufferInit(&priorityQueue->queues[i],
                             priorityQueue->buffers[i],
                             sizeof(priorityQueue->buffers[i]));
    }

    priorityQueue->bitmask = 0;
}

static
inline
UINT
Log2
    (
        UINT v
    )
{
    // This code is taken from:
    // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
    static const int MultiplyDeBruijnBitPosition[32] =
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
        UINT v
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
RtPriorityQueueAdd
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        IN TASK_DESCRIPTOR* td
    )
{
    UINT index = Power2Log2(td->priority);
    RT_CIRCULAR_BUFFER* queue = &priorityQueue->queues[index];
    RT_STATUS status = RtCircularBufferAdd(queue, &td, sizeof(td));

    if(RT_SUCCESS(status))
    {
        priorityQueue->bitmask |= td->priority;
    }

    return status;
}

RT_STATUS
RtPriorityQueueGet
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        OUT TASK_DESCRIPTOR** td
    )
{
    if(priorityQueue->bitmask)
    {
        UINT index = Log2(priorityQueue->bitmask);
        RT_CIRCULAR_BUFFER* queue = &priorityQueue->queues[index];

        return RtCircularBufferGet(queue, td, sizeof(*td));
    }
    else
    {
        return STATUS_NOT_FOUND;
    }
}

inline
RT_STATUS
RtPriorityQueueRemove
    (
        IN RT_PRIORITY_QUEUE* priorityQueue
    )
{
    if(priorityQueue->bitmask)
    {
        UINT index = Log2(priorityQueue->bitmask);
        RT_CIRCULAR_BUFFER* queue = &priorityQueue->queues[index];
        RT_STATUS status = RtCircularBufferRemove(queue, sizeof(TASK_DESCRIPTOR*));

        if(RT_SUCCESS(status) &&
           RtCircularBufferIsEmpty(queue))
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
