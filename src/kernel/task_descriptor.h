#pragma once

#include "rt.h"
#include "stack.h"

#define NUM_TASK_DESCRIPTORS    64

typedef
VOID
(*TASK_START_FUNC)
    (
        VOID
    );

typedef enum _TASK_PRIORITY
{
    SystemPriority = 0, // Reserved for system tasks (e.g. init task)
    HighPriority, // Interrupt tasks
    MediumPriority, // Interactive tasks
    LowPriority, // Computation tasks
    IdlePriority, // Reserved for the idle task
    NumPriority
} TASK_PRIORITY;

typedef enum _TASK_STATE
{
    Ready = 0,
    Running,
    Zombie
} TASK_STATE;

typedef struct _TASK_DESCRIPTOR
{
    INT taskId;
    INT parentTaskId;
    TASK_STATE state;
    TASK_PRIORITY priority;
    TASK_START_FUNC startFunc;
    UINT* stackPointer;
    STACK stack;
} TASK_DESCRIPTOR;

#define TaskGetStackPointer(task) ((task)->stackPointer)
#define TaskGetPriority(task) ((task)->priority)
#define TaskGetState(task) ((task)->state)
#define TaskGetTaskId(task) ((task)->taskId)
#define TaskGetParentTaskId(task) ((task)->parentTaskId)

VOID
TaskDescriptorInit
    (
        VOID
    );

RT_STATUS
TaskDescriptorCreate
    (
        IN INT parentTaskId,
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc,
        IN STACK* stack,
        OUT TASK_DESCRIPTOR** td
    );

RT_STATUS
TaskDescriptorDestroy
    (
        IN INT taskId
    );

RT_STATUS
TaskDescriptorGet
    (
        IN INT taskId,
        OUT TASK_DESCRIPTOR** td
    );
