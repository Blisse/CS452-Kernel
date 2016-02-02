#pragma once

#include <rtosc/buffer.h>
#include "rt.h"
#include "stack.h"

#define NUM_TASK_DESCRIPTORS 64
#define NUM_PRIORITY 32

typedef
VOID
(*TASK_START_FUNC)
    (
        VOID
    );

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
    UINT* stackPointer;
    INT taskId;
    INT parentTaskId;
    UINT priority;
    TASK_STATE state;
    RT_CIRCULAR_BUFFER mailbox;
    STACK* stack;    
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
