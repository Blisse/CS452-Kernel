#pragma once

#include "rt.h"

#define PRIORITY_SYSTEM 0 // Reserved for system tasks (e.g. init task)
#define PRIORITY_HIGH 1 // Interrupt tasks
#define PRIORITY_MEDIUM 2 // Interactive tasks
#define PRIORITY_LOW 3 // Computation tasks
#define PRIORITY_IDLE 4 // Reserved for the idle task

typedef
VOID
(*TASK_START_FUNC)
    (
        VOID
    );

typedef enum _TASK_STATE 
{
    Ready = 0,
    Running,
    Zombie
} TASK_STATE;

typedef struct _TASK_DESCRIPTOR 
{
    UINT* stack;
    TASK_STATE state;
    INT tid;
    INT parentTid;
    INT priority;
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
