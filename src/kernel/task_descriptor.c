#include "task_descriptor.h"

static TASK_DESCRIPTOR g_taskDescriptors[NUM_TASK_DESCRIPTORS];
static INT g_taskDescriptorIds[NUM_TASK_DESCRIPTORS + 1];
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

    for(i = 0; i < NUM_TASK_DESCRIPTORS && RT_SUCCESS(status); i++)
    {
        TASK_DESCRIPTOR* td = &g_taskDescriptors[i];

        // Initialize to -1 incase someone tries to send this task a message
        td->taskId = -1;

        status = RtCircularBufferAdd(&g_taskDescriptorIdQueue, &i, sizeof(i));
    }

    return status;
}

inline
RT_STATUS
TaskDescriptorAllocate
    (
        OUT TASK_DESCRIPTOR** td
    )
{
    INT taskId;
    RT_STATUS status = RtCircularBufferGetAndRemove(&g_taskDescriptorIdQueue, 
                                                    &taskId, 
                                                    sizeof(taskId));

    if (RT_SUCCESS(status))
    {
        TASK_DESCRIPTOR* newTd = &g_taskDescriptors[taskId % NUM_TASK_DESCRIPTORS];

        newTd->taskId = taskId;

        *td = newTd;        
    }

    return status;
}

inline
RT_STATUS
TaskDescriptorDeallocate
    (
        IN TASK_DESCRIPTOR* td
    )
{
    INT newTaskId = td->taskId + NUM_TASK_DESCRIPTORS;

    return RtCircularBufferAdd(&g_taskDescriptorIdQueue, &newTaskId, sizeof(newTaskId));
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

inline
RT_STATUS
TaskDescriptorGet
    (
        IN INT taskId,
        OUT TASK_DESCRIPTOR** td
    )
{
    if(TaskDescriptorpIsValidId(taskId))
    {
        TASK_DESCRIPTOR* getTd = &g_taskDescriptors[taskId % NUM_TASK_DESCRIPTORS];

        if (getTd->taskId == taskId)
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
