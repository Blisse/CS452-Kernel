#include <rtosc/assert.h>
#include <rtosc/string.h>
#include <rtos.h>

CHAR
ReadChar
    (
        IN IO_DEVICE* device
    )
{
    CHAR c;

    VERIFY(SUCCESSFUL(Read(device, &c, sizeof(c))));

    return c;
}

inline
INT
WriteChar
    (
        IN IO_DEVICE* device, 
        IN CHAR c
    )
{
    return Write(device, &c, sizeof(c));
}

INT
ReadString
    (
        IN IO_DEVICE* device, 
        IN STRING buffer, 
        IN UINT bufferLength
    )
{
    return Read(device, buffer, bufferLength);
}

inline
INT
WriteString
    (
        IN IO_DEVICE* device, 
        IN STRING str
    )
{
    return Write(device, str, RtStrLen(str));
}

INT
WriteFormattedString
    (
        IN IO_DEVICE* device, 
        IN STRING str, 
        ...
    )
{
    // TODO
    // Cannibalize bwio and use the other functions in this file
    // That's why the other write functions in this file are inlined
    ASSERT(FALSE);
    return -1;
}
