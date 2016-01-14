#pragma once

#include "buffer.h"
#include "object.h"
#include "rt.h"

typedef enum _IO_DEVICE_STATUS
{
    IoReady = 0,
    IoWritePending
} IO_DEVICE_STATUS;

struct _IO_DEVICE;
typedef struct _IO_DEVICE IO_DEVICE;

typedef
RT_STATUS
(*IO_DEVICE_WRITE_FUNC)
    (
        IN IO_DEVICE* device,
        IN CHAR byte
    );

struct _IO_DEVICE
{
    IO_DEVICE_STATUS status;
    IO_DEVICE_WRITE_FUNC write;
    RT_CIRCULAR_BUFFER readBuffer;
    RT_CIRCULAR_BUFFER writeBuffer;
};

VOID
IoDeviceInit
    (
        IN IO_DEVICE* device,
        IN HANDLE handle,
        IN IO_DEVICE_WRITE_FUNC writeFunc,
        IN PVOID readBuffer,
        IN UINT readBufferSize,
        IN PVOID writeBuffer,
        IN UINT writeBufferSize
    );

#define IoDeviceIsReady(device) (IoReady == (device)->status)
#define IoDeviceIsWriting(device) (IoWritePending == (device)->status)

RT_STATUS
IoWrite
    (
        IN IO_DEVICE* device,
        IN PVOID buffer,
        IN UINT length
    );

VOID
IoWriteFinished
    (
        IN IO_DEVICE* device
    );

// Temporary for A0
VOID
IoFlush
    (
        IN IO_DEVICE* device
    );

RT_STATUS
IoRead
    (
        IN IO_DEVICE* device,
        IN PVOID buffer,
        IN UINT requestedBytes,
        OUT UINT* actualBytes
    );

VOID
IoReceiveData
    (
        IN IO_DEVICE* device,
        IN PVOID buffer,
        IN UINT length
    );
