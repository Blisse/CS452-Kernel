#pragma once

#include "rt.h"

/************************************
 *          TASK API                *
 ************************************/

#define IDLE_PRIORITY 0x1 /* Reserved for idle task */
#define PRIORITY_1 0x2
#define PRIORITY_2 0x4
#define PRIORITY_3 0x8
#define PRIORITY_4 0x10
#define PRIORITY_5 0x20
#define PRIORITY_6 0x40
#define PRIORITY_7 0x80
#define PRIORITY_8 0x100
#define PRIORITY_9 0x200
#define PRIORITY_10 0x400
#define PRIORITY_11 0x800
#define PRIORITY_12 0x1000
#define PRIORITY_13 0x2000
#define PRIORITY_14 0x4000
#define PRIORITY_15 0x8000
#define PRIORITY_16 0x10000
#define PRIORITY_17 0x20000
#define PRIORITY_18 0x40000
#define PRIORITY_19 0x80000
#define PRIORITY_20 0x100000
#define PRIORITY_21 0x200000
#define PRIORITY_22 0x400000
#define PRIORITY_23 0x800000
#define PRIORITY_24 0x1000000
#define PRIORITY_25 0x2000000
#define PRIORITY_26 0x4000000
#define PRIORITY_27 0x8000000
#define PRIORITY_28 0x10000000
#define PRIORITY_29 0x20000000
#define PRIORITY_30 0x40000000
#define SYSTEM_PRIORITY 0x80000000 /* Reserved for system tasks */

#define SUCCESSFUL(x) ((x) >= 0)

extern
INT
Create
    (
        IN UINT priority, 
        IN VOID (*code)()
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

/************************************
 *          IPC API                 *
 ************************************/

extern
INT
Send
    (
        IN INT taskId,
        IN PVOID message,
        IN INT messageLength,
        IN PVOID reply,
        IN INT replyLength
    );

extern
INT
Receive
    (
        OUT INT* taskId,
        OUT PVOID message,
        IN INT messageLength
    );

extern
INT
Reply
    (
        IN INT taskId,
        IN PVOID reply,
        IN INT replyLength
    );
