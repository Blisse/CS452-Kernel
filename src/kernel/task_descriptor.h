#pragma once

#include <rtosc/buffer.h>
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
    ReadyState = 0,
    RunningState,
    SendBlockedState,
    ReceiveBlockedState,
    ReplyBlockedState,
    ZombieState
} TASK_STATE;

typedef struct _TASK_MESSAGE {
    INT senderId;
    PVOID message;
    INT messageLength;
} TASK_MESSAGE;

typedef struct _TASK_DESCRIPTOR {
    INT taskId;
    INT parentTaskId;
    TASK_STATE state;
    TASK_PRIORITY priority;
    UINT* stackPointer;
    STACK stack;

    INT* receiveMessageSenderId;
    PVOID receiveMessageBuffer;
    INT receiveMessageLength;

    PVOID replyMessageBuffer;
    INT replyMessageLength;

    TASK_MESSAGE messagesBuffer[NUM_TASK_DESCRIPTORS];
    RT_CIRCULAR_BUFFER messages;

} TASK_DESCRIPTOR;

#define TaskGetStackPointer(task) ((task)->stackPointer)
#define TaskGetPriority(task) ((task)->priority)
#define TaskGetState(task) ((task)->state)
#define TaskGetTaskId(task) ((task)->taskId)
#define TaskGetParentTaskId(task) ((task)->parentTaskId)

RT_STATUS
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

inline
TASK_DESCRIPTOR*
TaskDescriptorIndex
    (
        IN INT taskId
    );

inline
RT_STATUS
TaskDescriptorGet
    (
        IN INT taskId,
        OUT TASK_DESCRIPTOR** td
    );

inline
RT_STATUS
TaskDescriptorMessagePush
    (
        IN TASK_DESCRIPTOR* receivingTd,
        IN INT senderId,
        IN PVOID message,
        IN INT messageLength
    );

inline
RT_STATUS
TaskDescriptorMessagePop
    (
        IN TASK_DESCRIPTOR* receivingTd,
        OUT TASK_MESSAGE* message
    );

inline
VOID
TaskDescriptorSetReturnValue
    (
        IN TASK_DESCRIPTOR* td,
        INT returnValue
    );
