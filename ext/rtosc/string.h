#pragma once

#include <rt.h>

INT
RtStrCmp
    (
        STRING str1,
        STRING str2
    );

BOOLEAN
RtStrEqual
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
RtStrPrintFormatted
    (
        OUT STRING ret,
        IN INT retLength,
        IN STRING fmt,
        ...
    );

VOID
RtMemcpy
    (
        PVOID dest,
        PVOID src,
        UINT bytes
    );

