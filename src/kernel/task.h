#pragma once

#include "rt.h"

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
    UINT* stack;
    TASK_PRIORITY priority;
    TASK_STATE state;
    INT tid;
    INT parentTid;
} TASK_DESCRIPTOR;

VOID
TaskInit
    (
        VOID
    );

INT
TaskCreate
    (
        IN INT priority, 
        IN TASK_START_FUNC start
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

INT
TaskGetCurrentTid
    (
        VOID
    );

INT
TaskGetCurrentParentTid
    (
        VOID
    );

VOID
TaskDestroyCurrent
    (
        VOID
    );
