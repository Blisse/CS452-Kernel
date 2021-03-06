#pragma once

#include <rt.h>
#include <rtkernel.h>
#include <rtosc/buffer.h>
#include "stack.h"

typedef enum _TASK_STATE
{
    ReadyState = 0,
    RunningState,
    SendBlockedState,
    ReceiveBlockedState,
    ReplyBlockedState,
    EventBlockedState,
    ZombieState
} TASK_STATE;

typedef struct _TASK_DESCRIPTOR {
    UINT* stackPointer;
    INT taskId;
    INT parentTaskId;
    TASK_PRIORITY priority;
    TASK_STATE state;
    RT_CIRCULAR_BUFFER mailbox;
    STACK* stack;
} TASK_DESCRIPTOR;

RT_STATUS
TaskDescriptorInit();

RT_STATUS
TaskDescriptorAllocate
    (
        OUT TASK_DESCRIPTOR** td
    );

RT_STATUS
TaskDescriptorDeallocate
    (
        IN TASK_DESCRIPTOR* td
    );

RT_STATUS
TaskDescriptorGet
    (
        IN INT taskId,
        OUT TASK_DESCRIPTOR** td
    );
