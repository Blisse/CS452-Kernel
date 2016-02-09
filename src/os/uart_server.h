#pragma once

#include "rt.h"

#define COM1_CHANNEL 0
#define COM2_CHANNEL 1

VOID
UartServerCreateTask
    (
        VOID
    );

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
