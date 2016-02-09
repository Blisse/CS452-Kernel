#pragma once

#include "rt.h"

#define SUCCESSFUL(x) ((x) >= 0)

/************************************
 *          TASK API                *
 ************************************/

#define NUM_TASKS 64

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
    SystemPriority = 0x80000000,
    NumPriority = 32
} TASK_PRIORITY;

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
        IN TASK_PRIORITY priority,
        IN TASK_START_FUNC code
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
 *          NAME API                *
 ************************************/

INT
RegisterAs
    (
        IN STRING name
    );

INT
WhoIs
    (
        IN STRING name
    );

/************************************
 *          EVENT API               *
 ************************************/

typedef enum _EVENT
{
    ClockEvent = 0,
    NumEvent
} EVENT;

extern
INT
AwaitEvent
    (
        EVENT event
    );

/************************************
 *          TIME API                *
 ************************************/

INT
Delay
    (
        IN INT ticks
    );

INT
Time
    (
        VOID
    );

INT
DelayUntil
    (
        IN INT ticks
    );

/************************************
 *          I/O API                 *
 ************************************/

#define COM1_CHANNEL 0
#define COM2_CHANNEL 1

INT
GetString
    (
        IN INT channel, 
        IN STRING buffer, 
        IN UINT bufferLength
    );

INT
PutString
    (
        IN INT channel, 
        IN STRING str
    );

INT
PutFormattedString
    (
        IN INT channel, 
        IN STRING str
    );

CHAR
Getc
    (
        IN INT channel
    );

INT
Putc
    (
        IN INT channel, 
        IN CHAR c
    );

/************************************
 *       SHUTDOWN API               *
 ************************************/

VOID
Shutdown
    (
        VOID
    );

/************************************
 *       PERFORMANCE API            *
 ************************************/

typedef struct _TASK_PERFORMANCE {
    UINT activeTicks;
} TASK_PERFORMANCE;

extern
INT
QueryPerformance
    (
        IN INT taskId,
        OUT TASK_PERFORMANCE* performance
    );
