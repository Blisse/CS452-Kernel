#pragma once

#include "rt.h"
#include "task_descriptor.h"

VOID
TaskInit
    (
        VOID
    );

RT_STATUS
TaskCreate
    (
        IN TASK_DESCRIPTOR* parent,
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC startFunc,
        OUT TASK_DESCRIPTOR** td
    );

VOID
TaskDestroy
    (
        IN TASK_DESCRIPTOR* td
    );

inline
BOOLEAN
TaskValidate
    (
        IN TASK_DESCRIPTOR* td
    );

inline
VOID
TaskSetReturnValue
    (
        IN TASK_DESCRIPTOR* td, 
        IN INT returnValue
    );

inline
VOID
TaskStoreAsyncParameter
    (
        IN TASK_DESCRIPTOR* td, 
        IN PVOID parameter, 
        IN UINT size
    );

inline
VOID
TaskRetrieveAsyncParameter
    (
        IN TASK_DESCRIPTOR* td, 
        IN PVOID parameter, 
        IN UINT size
    );
