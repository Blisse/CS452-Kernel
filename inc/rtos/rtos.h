#pragma once

#include "rt.h"

#define SUCCESSFUL(x) ((x) >= 0)

/************************************
 *          TASK API                *
 ************************************/

typedef enum _TASK_PRIORITY {
    IdlePriority = 0x1,
    LowestPriority = 0x2,
    Priority2 = 0x4,
    Priority3 = 0x8,
    Priority4 = 0x10,
    Priority5 = 0x20,
    Priority6 = 0x40,
    Priority7 = 0x80,
    Priority8 = 0x100,
    Priority9 = 0x200,
    Priority10 = 0x400,
    Priority11 = 0x800,
    Priority12 = 0x1000,
    Priority13 = 0x2000,
    Priority14 = 0x4000,
    Priority15 = 0x8000,
    Priority16 = 0x10000,
    Priority17 = 0x20000,
    Priority18 = 0x40000,
    Priority19 = 0x80000,
    Priority20 = 0x100000,
    Priority21 = 0x200000,
    Priority22 = 0x400000,
    Priority23 = 0x800000,
    Priority24 = 0x1000000,
    Priority25 = 0x2000000,
    Priority26 = 0x4000000,
    Priority27 = 0x8000000,
    Priority28 = 0x10000000,
    Priority29 = 0x20000000,
    HighestPriority = 0x40000000,
    SystemPriority = 0x80000000
} TASK_PRIORITY;

extern
INT
Create
    (
        IN TASK_PRIORITY priority,
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

/************************************
 *          EVENT API               *
 ************************************/

#define EVENT_CLOCK 0

 extern
 INT
 AwaitEvent
    (
        INT eventType
    );
