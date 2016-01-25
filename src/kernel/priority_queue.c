#include "priority_queue.h"

VOID
RtPriorityQueueInit
    (
        IN RT_PRIORITY_QUEUE* priorityQueue
    )
{
    UINT i;

    for(i = 0; i < NumPriority; i++)
    {
        RtCircularBufferInit(&priorityQueue->queues[i], 
                             priorityQueue->buffers[i], 
                             sizeof(priorityQueue->buffers[i]));
    }
}

inline
RT_STATUS
RtPriorityQueueAdd
    (
        IN RT_PRIORITY_QUEUE* priorityQueue, 
        IN TASK_DESCRIPTOR* td
    )
{
    return RtCircularBufferAdd(&priorityQueue->queues[td->priority], 
                               &td, 
                               sizeof(td));
}

RT_STATUS
RtPriorityQueueGet
    (
        IN RT_PRIORITY_QUEUE* priorityQueue, 
        OUT TASK_DESCRIPTOR** td
    )
{
    UINT i;

    for(i = 0; i < NumPriority; i++)
    {
        RT_CIRCULAR_BUFFER* queue = &priorityQueue->queues[i];

        if(!RtCircularBufferIsEmpty(queue))
        {
            return RtCircularBufferGet(queue, 
                                       td, 
                                       sizeof(*td));
        }
    }

    return STATUS_NOT_FOUND;
}

inline
RT_STATUS
RtPriorityQueueRemove
    (
        IN RT_PRIORITY_QUEUE* priorityQueue, 
        IN TASK_DESCRIPTOR* td
    )
{
    return RtCircularBufferRemove(&priorityQueue->queues[td->priority],
                                  sizeof(td));
}
