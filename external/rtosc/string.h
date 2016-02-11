#pragma once

#include <rt.h>

INT
RtStrCmp
    (
        STRING str1,
        STRING str2
    );

UINT
RtStrLen
    (
        STRING str
    );

INT
RtAtoi
    (
        STRING str
    );

STRING
RtStrFind
    (
        STRING str,
        STRING subStr
    );


void RtUitoa( unsigned int num, char *bf, unsigned int* len );

#define RtStrEqual(str1, str2) (0 == RtStrCmp(str1, str2))

VOID
RtMemcpy
    (
        PVOID dest,
        PVOID src,
        UINT bytes
    );

