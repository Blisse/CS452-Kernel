#include "scheduler.h"

#include <rtosc/buffer.h>

static TASK_DESCRIPTOR* g_CurrentTask;

static TASK_DESCRIPTOR* g_underlyingBuffers[NumPriority][NUM_TASKS];
static RT_CIRCULAR_BUFFER g_priorityQueue[NumPriority];

VOID
SchedulerInit
    (
        VOID
    )
{
    UINT i;
    g_CurrentTask = NULL;

    for(i = 0; i < NumPriority; i++)
    {
        RtCircularBufferInit(&g_priorityQueue[i], 
                             g_underlyingBuffers[i], 
                             sizeof(g_underlyingBuffers[i]));
    }
}

inline
RT_STATUS
SchedulerAddTask
    (
        IN TASK_DESCRIPTOR* task
    )
{
    return RtCircularBufferAdd(&g_priorityQueue[task->priority], 
                               task, 
                               sizeof(task));
}

static
RT_STATUS
SchedulerpFindNextTask
    (
        OUT TASK_DESCRIPTOR** nextTask
    )
{
    UINT i;
    RT_STATUS status = STATUS_NOT_FOUND;

    for(i = 0; i < NumPriority; i++)
    {
        if(!RtCircularBufferIsEmpty(&g_priorityQueue[i]))
        {
            status = RtCircularBufferGetAndRemove(&g_priorityQueue[i], 
                                                  nextTask, 
                                                  sizeof(*nextTask));
            break;
        }
    }

    return status;
}

RT_STATUS
SchedulerGetNextTask
    (
        OUT TASK_DESCRIPTOR** task
    )
{   
    TASK_DESCRIPTOR* nextTask;
    RT_STATUS status = SchedulerpFindNextTask(&nextTask);    

    // If there's nothing in the queue, that's OK
    // We'll try to re-run the current task
    if(STATUS_NOT_FOUND == status)
    {
        nextTask = NULL;
        status = STATUS_SUCCESS;
    }

    if(RT_SUCCESS(status))
    {
        if(NULL != nextTask)
        {
            if(Zombie != TaskGetState(g_CurrentTask))
            {
                if(TaskGetPriority(nextTask) <= TaskGetPriority(g_CurrentTask))
                {
                    status = SchedulerAddTask(g_CurrentTask);

                    if(RT_SUCCESS(status))
                    {
                        g_CurrentTask = nextTask;
                    }
                }
            }
            else
            {
                g_CurrentTask = nextTask;
            }
        }
        else if(Zombie == TaskGetState(g_CurrentTask))
        {
            g_CurrentTask = NULL;
            status = STATUS_NOT_FOUND;
        }

        *task = g_CurrentTask;
    }    
    
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
