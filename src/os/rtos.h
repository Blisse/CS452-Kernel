#pragma once

#include "rt.h"

/************************************
 *          TASK API                *
 ************************************/

typedef
VOID
(*TASK_START_FUNC)
    (
        VOID
    );

extern
RT_STATUS
Create
    (
        IN INT priority, 
        IN TASK_START_FUNC start
    );

extern
RT_STATUS
MyTid
    (
        VOID
    );

extern
RT_STATUS
MyParentTid
    (
        VOID
    );

extern
VOID
Pass
    (
        VOID
    );

extern
VOID
Exit
    (
        VOID
    );
