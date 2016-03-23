#pragma once

#include <rt.h>

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
 *          TIME API                *
 ************************************/

INT
Delay
    (
        IN INT ticks
    );

INT
Time();

INT
DelayUntil
    (
        IN INT ticks
    );

/************************************
 *       I/O SERVER API             *
 ************************************/

typedef enum _IO_DEVICE_TYPE
{
    UartDevice = 0,
    NumDeviceType
} IO_DEVICE_TYPE;

typedef enum _IO_CHANNEL
{
    ChannelCom1 = 0, 
    ChannelCom2, 
    NumChannel
} IO_CHANNEL;

typedef struct _IO_DEVICE
{
    INT readTaskId;
    INT writeTaskId;
} IO_DEVICE;

INT
Open
    (
        IN IO_DEVICE_TYPE type, 
        IN IO_CHANNEL channel, 
        OUT IO_DEVICE* device
    );

INT
Read
    (
        IN IO_DEVICE* device, 
        IN PVOID buffer, 
        IN UINT bufferLength
    );

INT
Write
    (
        IN IO_DEVICE* device, 
        IN PVOID buffer,
        IN UINT bufferLength
    );

INT
FlushInput
    (
        IN IO_DEVICE* device
    );


/************************************
 *         I/O LIBRARY API          *
 ************************************/

CHAR
ReadChar
    (
        IN IO_DEVICE* device
    );

INT
WriteChar
    (
        IN IO_DEVICE* device, 
        IN CHAR c
    );

INT
ReadString
    (
        IN IO_DEVICE* device, 
        IN STRING buffer, 
        IN UINT bufferLength
    );

INT
WriteString
    (
        IN IO_DEVICE* device, 
        IN STRING str
    );

INT
WriteFormattedString
    (
        IN IO_DEVICE* device, 
        IN STRING str, 
        ...
    );

/************************************
 *       SHUTDOWN API               *
 ************************************/

typedef
VOID
(*SHUTDOWN_HOOK_FUNC)
    (
        VOID
    );

INT
ShutdownRegisterHook
    (
        IN SHUTDOWN_HOOK_FUNC shutdownFunc
    );

VOID
Shutdown();

/************************************
 *            INIT TASK             *
 ************************************/

VOID
InitOsTasks();
