#include "scheduler.h"

#include "priority_queue.h"

static TASK_DESCRIPTOR* g_CurrentTask;
static RT_PRIORITY_QUEUE g_priorityQueue;

VOID
SchedulerInit
    (
        VOID
    )
{
    g_CurrentTask = NULL;

    RtPriorityQueueInit(&g_priorityQueue);
}

inline
RT_STATUS
SchedulerAddTask
    (
        IN TASK_DESCRIPTOR* task
    )
{
    return RtPriorityQueueAdd(&g_priorityQueue, task);
}

RT_STATUS 
SchedulerGetNextTask
    (
        OUT TASK_DESCRIPTOR** task
    )
{
    TASK_DESCRIPTOR* nextTask;
    RT_STATUS status = RtPriorityQueueGet(&g_priorityQueue, &nextTask);

    if(NULL != g_CurrentTask && Zombie != TaskGetState(g_CurrentTask))
    {
        if(RT_SUCCESS(status) && 
           TaskGetPriority(nextTask) <= TaskGetPriority(g_CurrentTask))
        {
            status = RtPriorityQueueRemove(&g_priorityQueue, nextTask);

            if(RT_SUCCESS(status))
            {
                status = SchedulerAddTask(g_CurrentTask);

                if(RT_SUCCESS(status))
                {
                    g_CurrentTask = nextTask;
                }
            }
        }
    }
    else if(RT_SUCCESS(status))
    {
        status = RtPriorityQueueRemove(&g_priorityQueue, nextTask);

        if(RT_SUCCESS(status))
        {
            g_CurrentTask = nextTask;
        }
    }

    *task = g_CurrentTask;

    return status;
}

inline
TASK_DESCRIPTOR*
SchedulerGetCurrentTask
    (
        VOID
    )
{
    return g_CurrentTask;
}

VOID
SchedulerPassCurrentTask
    (
        VOID
    )
{
    // This is intentionally left blank - it is a NOP
}
