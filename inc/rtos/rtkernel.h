#pragma once

#include <rt.h>

/************************************
 *          TASK API                *
 ************************************/

#define NUM_TASKS 64

// All priorities are a power of 2 so that
// the scheduler can operate in O(1)
typedef enum _TASK_PRIORITY {
    IdlePriority = 0x1,
    LowestUserPriority = 0x2,
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
    HighestUserPriority = 0x4000000,
    LowestSystemPriority = 0x8000000,
    Priority28 = 0x10000000,
    Priority29 = 0x20000000,
    Priority30 = 0x40000000,
    HighestSystemPriority = 0x80000000,
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
MyTid();

extern
INT
MyParentTid();

extern
VOID
Pass();

extern
VOID
Exit();

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

typedef enum _EVENT
{
    ClockEvent = 0,
    UartCom1ReceiveEvent,
    UartCom1TransmitEvent,
    UartCom2ReceiveEvent,
    UartCom2TransmitEvent,
    NumEvent
} EVENT;

extern
INT
AwaitEvent
    (
        EVENT event
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
