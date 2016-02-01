#pragma once

#include <rtosc/buffer.h>
#include "rt.h"
#include "stack.h"

#define NUM_TASK_DESCRIPTORS 64

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
    ReadyState = 0,
    RunningState,
    SendBlockedState,
    ReceiveBlockedState,
    ReplyBlockedState,
    ZombieState
} TASK_STATE;

typedef struct _TASK_DESCRIPTOR {
    INT taskId;
    RT_CIRCULAR_BUFFER mailbox;
    TASK_STATE state;
    TASK_PRIORITY priority;
    UINT* stackPointer;
    STACK* stack;
    INT parentTaskId;
} TASK_DESCRIPTOR;

#define TaskDescriptorGetStackPointer(task) ((task)->stackPointer)
#define TaskDescriptorGetPriority(task) ((task)->priority)
#define TaskDescriptorGetState(task) ((task)->state)
#define TaskDescriptorGetTaskId(task) ((task)->taskId)
#define TaskDescriptorGetParentTaskId(task) ((task)->parentTaskId)

RT_STATUS
TaskDescriptorInit
    (
        VOID
    );

inline
RT_STATUS
TaskDescriptorAllocate
    (
        OUT TASK_DESCRIPTOR** td
    );

inline
RT_STATUS
TaskDescriptorDeallocate
    (
        IN TASK_DESCRIPTOR* td
    );

inline
RT_STATUS
TaskDescriptorGet
    (
        IN INT taskId,
        OUT TASK_DESCRIPTOR** td
    );
