#include "scheduler.h"

#include <rtosc/priority_queue.h>
#include <rtos.h>

#include "task_descriptor.h"

static TASK_DESCRIPTOR* g_currentTd;

static TASK_DESCRIPTOR* g_taskDescriptorsPriorityQueueData[NumPriority][NUM_TASK_DESCRIPTORS];
static RT_CIRCULAR_BUFFER g_taskDescriptorPriorityQueueBuffer[NumPriority];
static RT_PRIORITY_QUEUE g_taskDescriptorPriorityQueue;

VOID
SchedulerInit
    (
        VOID
    )
{
    g_currentTd = NULL;

    RtPriorityQueueInit(&g_taskDescriptorPriorityQueue,
                        g_taskDescriptorsPriorityQueueData,
                        g_taskDescriptorPriorityQueueBuffer,
                        sizeof(TASK_DESCRIPTOR*),
                        NumPriority,
                        NUM_TASK_DESCRIPTORS
                        );
}

inline
RT_STATUS
SchedulerAddTask
    (
        IN TASK_DESCRIPTOR* td
    )
{
    return RtPriorityQueuePush(&g_taskDescriptorPriorityQueue, td->priority, &td, sizeof(td));
}

RT_STATUS
SchedulerGetNextTask
    (
        OUT TASK_DESCRIPTOR** td
    )
{
    TASK_DESCRIPTOR* nextTd;

    RT_STATUS status = RtPriorityQueuePeek(&g_taskDescriptorPriorityQueue, &nextTd, sizeof(nextTd));

    if(NULL != g_currentTd && ReadyState == g_currentTd->state)
    {
        if(RT_SUCCESS(status) && TaskDescriptorPriorityIsHigherOrEqual(nextTd, g_currentTd))
        {
            status = RtPriorityQueuePop(&g_taskDescriptorPriorityQueue, sizeof(nextTd));

            if(RT_SUCCESS(status))
            {
                status = SchedulerAddTask(g_currentTd);

                if(RT_SUCCESS(status))
                {
                    g_currentTd = nextTd;
                }
            }
        }
        else
        {
            // Reschedule the current task
            status = STATUS_SUCCESS;
        }
    }
    else if(RT_SUCCESS(status))
    {
        status = RtPriorityQueuePop(&g_taskDescriptorPriorityQueue, sizeof(nextTd));

        if(RT_SUCCESS(status))
        {
            g_currentTd = nextTd;
        }
    }

    *td = g_currentTd;

    return status;
}

inline
TASK_DESCRIPTOR*
SchedulerGetCurrentTask
    (
        VOID
    )
{
    return g_currentTd;
}
