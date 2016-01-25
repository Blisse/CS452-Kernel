#pragma once

#include "rt.h"

/************************************
 *          TASK API                *
 ************************************/

#define HIGH_PRIORITY 1
#define MEDIUM_PRIORITY 2
#define LOW_PRIORITY 3

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
