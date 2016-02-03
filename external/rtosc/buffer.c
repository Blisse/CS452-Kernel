#include "buffer.h"
#include "string.h"

VOID
RtCircularBufferInit
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID underlyingBuffer,
        IN UINT capacity
    )
{
    buffer->underlyingBuffer = underlyingBuffer;
    buffer->capacity = capacity;
    buffer->front = 0;
    buffer->back = 0;
    buffer->size = 0;
}

RT_STATUS
RtCircularBufferPush
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID sourceBuffer,
        IN UINT bytesToAdd
    )
{
    RT_STATUS status;

    if((buffer->size + bytesToAdd) <= buffer->capacity)
    {
        UINT newBack = (buffer->back + bytesToAdd) % buffer->capacity;

        if(newBack > buffer->back)
        {
            // Don't need to wrap around the buffer - perform a direct copy
            RtMemcpy(ptr_add(buffer->underlyingBuffer, buffer->back), sourceBuffer, bytesToAdd);
        }
        else
        {
            // Need to wrap around the buffer - perform 2 copies
            UINT bytesTillEnd = buffer->capacity - buffer->back;

            // Copy till the end of the buffer
            RtMemcpy(ptr_add(buffer->underlyingBuffer, buffer->back), sourceBuffer, bytesTillEnd);

            // Copy the remainder from the beginning of the buffer
            RtMemcpy(buffer->underlyingBuffer, ptr_add(sourceBuffer, bytesTillEnd), newBack);
        }

        buffer->back = newBack;
        buffer->size += bytesToAdd;

        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_BUFFER_OVERFLOW;
    }

    return status;
}

RT_STATUS
RtCircularBufferPeek
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID targetBuffer,
        IN UINT bytesToRemove
    )
{
    RT_STATUS status;

    if(bytesToRemove <= buffer->size)
    {
        UINT newFront = (buffer->front + bytesToRemove) % buffer->capacity;

        if(newFront > buffer->front)
        {
            // Don't need to wrap around the buffer - perform a direct copy
            RtMemcpy(targetBuffer, ptr_add(buffer->underlyingBuffer, buffer->front), bytesToRemove);
        }
        else
        {
            // Need to wrap around the buffer - perform 2 copies
            UINT bytesTillEnd = buffer->capacity - buffer->front;

            // Copy till the end of the buffer
            RtMemcpy(targetBuffer, ptr_add(buffer->underlyingBuffer, buffer->front), bytesTillEnd);

            // Copy the remainder from the beginning of the buffer
            RtMemcpy(ptr_add(targetBuffer, bytesTillEnd), buffer->underlyingBuffer, newFront);
        }

        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_BUFFER_TOO_SMALL;
    }

    return status;
}

inline
RT_STATUS
RtCircularBufferPop
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN UINT bytesToRemove
    )
{
    if(bytesToRemove <= buffer->size)
    {
        buffer->front = (buffer->front + bytesToRemove) % buffer->capacity;
        buffer->size -= bytesToRemove;

        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
}

RT_STATUS
RtCircularBufferPeekAndPop
    (
        IN RT_CIRCULAR_BUFFER* buffer,
        IN PVOID targetBuffer,
        IN UINT bytesToRemove
    )
{
    RT_STATUS status = RtCircularBufferPeek(buffer, targetBuffer, bytesToRemove);

    if(RT_SUCCESS(status))
    {
        status = RtCircularBufferPop(buffer, bytesToRemove);
    }

    return status;
}

inline
BOOLEAN
RtCircularBufferIsEmpty
    (
        IN RT_CIRCULAR_BUFFER* buffer
    )
{
    return (buffer->size == 0);
}

inline
BOOLEAN
RtCircularBufferIsFull
    (
        IN RT_CIRCULAR_BUFFER* buffer
    )
{
    return (buffer->size == buffer->capacity);
}

inline
UINT
RtCircularBufferSize
    (
        IN RT_CIRCULAR_BUFFER* buffer
    )
{
    return buffer->size;
}
