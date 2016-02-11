#pragma once

#include "buffer.h"
#include <rt.h>

typedef struct _RT_PRIORITY_QUEUE
{
    PVOID buffers;
    RT_CIRCULAR_BUFFER* queues;
    UINT bitmask;
} RT_PRIORITY_QUEUE;

VOID
RtPriorityQueueInit
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        IN PVOID buffers,
        IN RT_CIRCULAR_BUFFER* queues,
        IN UINT typeSize,
        IN UINT priorities,
        IN UINT capacity
    );

RT_STATUS
RtPriorityQueuePush
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        IN UINT priority,
        IN PVOID sourceBuffer,
        IN UINT bytesToAdd
    );

RT_STATUS
RtPriorityQueuePeek
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        OUT PVOID targetBuffer,
        IN UINT bytesToGet
    );

RT_STATUS
RtPriorityQueuePop
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        IN UINT bytesToRemove
    );

RT_STATUS
RtPriorityQueuePeekAndPop
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        OUT PVOID targetBuffer,
        IN UINT bytesToGet
    );
