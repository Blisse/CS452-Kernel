#include "task_descriptor.h"

static TASK_DESCRIPTOR g_taskDescriptors[NUM_TASKS];
static INT g_taskDescriptorIds[NUM_TASKS];
static RT_CIRCULAR_BUFFER g_taskDescriptorIdQueue;

RT_STATUS
TaskDescriptorInit
    (
        VOID
    )
{
    UINT i;
    RT_STATUS status = STATUS_SUCCESS;

    RtCircularBufferInit(&g_taskDescriptorIdQueue, g_taskDescriptorIds, sizeof(g_taskDescriptorIds));

    for(i = 0; i < NUM_TASKS && RT_SUCCESS(status); i++)
    {
        TASK_DESCRIPTOR* td = &g_taskDescriptors[i];

        // Initialize to -1 incase someone tries to send this task a message
        td->taskId = -1;

        status = RtCircularBufferPush(&g_taskDescriptorIdQueue, &i, sizeof(i));
    }

    return status;
}

RT_STATUS
TaskDescriptorAllocate
    (
        OUT TASK_DESCRIPTOR** td
    )
{
    INT taskId;
    RT_STATUS status = RtCircularBufferPeekAndPop(&g_taskDescriptorIdQueue,
                                                    &taskId,
                                                    sizeof(taskId));

    if (RT_SUCCESS(status))
    {
        TASK_DESCRIPTOR* newTd = &g_taskDescriptors[taskId % NUM_TASKS];

        newTd->taskId = taskId;

        *td = newTd;        
    }

    return status;
}

RT_STATUS
TaskDescriptorDeallocate
    (
        IN TASK_DESCRIPTOR* td
    )
{
    INT newTaskId = td->taskId + NUM_TASKS;

    return RtCircularBufferPush(&g_taskDescriptorIdQueue, &newTaskId, sizeof(newTaskId));
}

inline
static
BOOLEAN
TaskDescriptorpIsValidId
    (
        IN INT taskId
    )
{
    return taskId >= 0;
}

RT_STATUS
TaskDescriptorGet
    (
        IN INT taskId,
        OUT TASK_DESCRIPTOR** td
    )
{
    if(likely(TaskDescriptorpIsValidId(taskId)))
    {
        TASK_DESCRIPTOR* getTd = &g_taskDescriptors[taskId % NUM_TASKS];

        if (likely(getTd->taskId == taskId))
        {
            *td = getTd;

            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_NOT_FOUND;
        }
    }
    else
    {
        return STATUS_INVALID_PARAMETER;
    }
}
