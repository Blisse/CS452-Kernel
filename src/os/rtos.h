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
INT
Create
    (
        IN INT priority, 
        IN TASK_START_FUNC start
    );

extern
INT
MyTid
    (
        VOID
    );

extern
INT
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
