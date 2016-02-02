#pragma once

#include <rtosc/buffer.h>
#include "rt.h"
#include "task.h"

typedef struct _RT_PRIORITY_QUEUE
{
    TASK_DESCRIPTOR* buffers[NUM_PRIORITY][NUM_TASK_DESCRIPTORS + 1];
    RT_CIRCULAR_BUFFER queues[NUM_PRIORITY];
    UINT bitmask;
} RT_PRIORITY_QUEUE;

VOID
RtPriorityQueueInit
    (
        IN RT_PRIORITY_QUEUE* priorityQueue
    );

inline
RT_STATUS
RtPriorityQueueAdd
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        IN TASK_DESCRIPTOR* td
    );

RT_STATUS
RtPriorityQueueGet
    (
        IN RT_PRIORITY_QUEUE* priorityQueue,
        OUT TASK_DESCRIPTOR** td
    );

inline
RT_STATUS
RtPriorityQueueRemove
    (
        IN RT_PRIORITY_QUEUE* priorityQueue
    );
