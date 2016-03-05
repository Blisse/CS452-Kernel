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
RtStrPrintFormattedVa
    (
        OUT STRING ret,
        IN INT retLength,
        IN STRING fmt,
        IN VA_LIST va
    );

INT
RtStrPrintFormatted
    (
        OUT STRING ret,
        IN INT retLength,
        IN STRING fmt,
        ...
    );

INT
RtStrConsumeToken
    (
        IN CHAR** str,
        OUT CHAR* buffer,
        IN INT bufferLength
    );

BOOLEAN
RtStrIsWhitespace
    (
        IN STRING str
    );

VOID
RtMemcpy
    (
        PVOID dest,
        PVOID src,
        UINT bytes
    );

VOID
RtMemset
    (
        PVOID dest,
        UINT size
    );
