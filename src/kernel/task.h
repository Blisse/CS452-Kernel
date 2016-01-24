#pragma once

#include "rt.h"
#include "task_descriptor.h"

<<<<<<< HEAD
#define NUM_TASKS 64

typedef
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
    UINT* stack;
    TASK_PRIORITY priority;
    TASK_STATE state;
    INT tid;
    INT parentTid;
} TASK_DESCRIPTOR;

#define TaskGetStack(task) ((task)->stack)
#define TaskGetPriority(task) ((task)->priority)
#define TaskGetState(task) ((task)->state)
#define TaskGetID(task) ((task)->tid)
#define TaskGetParentID(task) ((task)->parentTid)

VOID
TaskInit
    (
        VOID
    );

INT
TaskCreate
    (
        IN INT parentTaskId,
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc,
        OUT TASK_DESCRIPTOR** taskDescriptor
    );

BOOLEAN
TaskValidate
    (
        IN TASK_DESCRIPTOR* task
    );

inline
VOID
TaskUpdate
    (
        IN TASK_DESCRIPTOR* task
    );
