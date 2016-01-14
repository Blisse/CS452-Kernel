#pragma once

#include "rt.h"

typedef struct _RT_CIRCULAR_BUFFER {
    PVOID underlyingBuffer;
    UINT capacity;
    UINT front;
    UINT back;
} RT_CIRCULAR_BUFFER;

VOID
RtCircularBufferInit
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID underlyingBuffer,
        IN UINT capacity
    );

RT_STATUS
RtCircularBufferAdd
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID sourceBuffer,
        IN UINT bytesToAdd
    );

RT_STATUS
RtCircularBufferGet
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID targetBuffer,
        IN UINT bytesToGet
    );

inline
RT_STATUS
RtCircularBufferRemove
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN UINT bytesToRemove
    );

RT_STATUS
RtCircularBufferGetAndRemove
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID targetBuffer,
        IN UINT bytesToRemove
    );

inline
BOOLEAN
RtCircularBufferIsEmpty
    (
        IN RT_CIRCULAR_BUFFER* buffer
    );

inline
BOOLEAN
RtCircularBufferIsFull
    (
        IN RT_CIRCULAR_BUFFER* buffer
    );

inline
UINT
RtCircularBufferSize
    (
        IN RT_CIRCULAR_BUFFER* buffer
    );
