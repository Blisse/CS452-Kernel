#include "scheduler.h"

#include "priority_queue.h"

static TASK_DESCRIPTOR* g_currentTd;
static RT_PRIORITY_QUEUE g_priorityQueue;

VOID
SchedulerInit
    (
        VOID
    )
{
    g_currentTd = NULL;

    RtPriorityQueueInit(&g_priorityQueue);
}

inline
RT_STATUS
SchedulerAddTask
    (
        IN TASK_DESCRIPTOR* td
    )
{
    return RtPriorityQueueAdd(&g_priorityQueue, td);
}

RT_STATUS 
SchedulerGetNextTask
    (
        OUT TASK_DESCRIPTOR** td
    )
{
    TASK_DESCRIPTOR* nextTd;
    RT_STATUS status = RtPriorityQueueGet(&g_priorityQueue, &nextTd);

    if(NULL != g_currentTd && ReadyState == TaskGetState(g_currentTd))
    {
        if(RT_SUCCESS(status) && 
           TaskGetPriority(nextTd) <= TaskGetPriority(g_currentTd))
        {
            status = RtPriorityQueueRemove(&g_priorityQueue, nextTd);

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
        status = RtPriorityQueueRemove(&g_priorityQueue, nextTd);

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
