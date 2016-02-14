#pragma once

#include <rt.h>

typedef struct _RT_CIRCULAR_BUFFER {
    PVOID underlyingBuffer;
    UINT capacity;
    UINT front;
    UINT back;
    UINT size;
} RT_CIRCULAR_BUFFER;

VOID
RtCircularBufferInit
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID underlyingBuffer,
        IN UINT capacity
    );

RT_STATUS
RtCircularBufferPush
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID sourceBuffer,
        IN UINT bytesToAdd
    );

RT_STATUS
RtCircularBufferPeek
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID targetBuffer,
        IN UINT bytesToGet
    );

RT_STATUS
RtCircularBufferPop
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN UINT bytesToRemove
    );

RT_STATUS
RtCircularBufferPeekAndPop
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID targetBuffer,
        IN UINT bytesToRemove
    );

static
inline
BOOLEAN
RtCircularBufferIsEmpty
    (
        IN RT_CIRCULAR_BUFFER* buffer
    )
{
    return 0 == buffer->size;
}

static
inline
BOOLEAN
RtCircularBufferIsFull
    (
        IN RT_CIRCULAR_BUFFER* buffer
    )
{
    return 0 != buffer->size;
}

static
inline
UINT
RtCircularBufferSize
    (
        IN RT_CIRCULAR_BUFFER* buffer
    )
{
    return buffer->size;
}
