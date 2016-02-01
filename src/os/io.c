#include "io.h"

#include <rtosc/assert.h>
#include "ts7200.h"

static
RT_STATUS
IopDeviceWrite
    (
        IN IO_DEVICE* device
    )
{
    CHAR byte;
    RT_STATUS status = RtCircularBufferPeek(&device->writeBuffer, &byte, sizeof(byte));

    if(RT_SUCCESS(status))
    {
        status = device->write(device, byte);

        if(RT_SUCCESS(status))
        {
            device->status = IoWritePending;
            status = RtCircularBufferPop(&device->writeBuffer, sizeof(byte));
        }
        else if(STATUS_DEVICE_NOT_READY == status)
        {
            // It's OK if the device isn't ready yet
            status = STATUS_SUCCESS;
        }
        else
        {
            ASSERT(FALSE, "Write failed");
        }
    }

    return status;
}

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
    )
{
    device->status = IoReady;
    device->write = writeFunc;
    RtCircularBufferInit(&device->readBuffer, readBuffer, readBufferSize);
    RtCircularBufferInit(&device->writeBuffer, writeBuffer, writeBufferSize);

    ObjectRegisterHandle(handle, device);
}

RT_STATUS
IoWrite
    (
        IN IO_DEVICE* device,
        IN PVOID buffer,
        IN UINT length
    )
{
    RT_STATUS status = RtCircularBufferPush(&device->writeBuffer, buffer, length);

    if(RT_SUCCESS(status) &&
       IoDeviceIsReady(device))
    {
        status = IopDeviceWrite(device);
    }

    return status;
}

VOID
IoWriteFinished
    (
        IN IO_DEVICE* device
    )
{
    ASSERT(IoDeviceIsWriting(device), "Write finished for device that is not writing\n");

    device->status = IoReady;

    if(!RtCircularBufferIsEmpty(&device->writeBuffer))
    {
        RT_STATUS status = IopDeviceWrite(device);
        ASSERT(RT_SUCCESS(status), "Buffered write to io device failed\n");
        UNREFERENCED_PARAMETER(status);
    }
}

// Temporary for A0
VOID
IoFlush
    (
        IN IO_DEVICE* device
    )
{
    if(IoDeviceIsReady(device) &&
       !RtCircularBufferIsEmpty(&device->writeBuffer))
    {
        RT_STATUS status = IopDeviceWrite(device);
        ASSERT(RT_SUCCESS(status), "IoFlush failed\n");
        UNREFERENCED_PARAMETER(status);
    }
}

RT_STATUS
IoRead
    (
        IN IO_DEVICE* device,
        IN PVOID buffer,
        IN UINT requestedBytes,
        OUT UINT* actualBytes
    )
{
    RT_STATUS status;

    if(!RtCircularBufferIsEmpty(&device->readBuffer))
    {
        UINT readBufferSize = RtCircularBufferSize(&device->readBuffer);
        UINT bytesToRead = min(requestedBytes, readBufferSize);

        status = RtCircularBufferPeekAndPop(&device->readBuffer, buffer, bytesToRead);

        if(RT_SUCCESS(status))
        {
            *actualBytes = bytesToRead;
        }
    }
    else
    {
        status = STATUS_DEVICE_NOT_READY;
    }

    return status;
}

VOID
IoReceiveData
    (
        IN IO_DEVICE* device,
        IN PVOID buffer,
        IN UINT length
    )
{
    RT_STATUS status = RtCircularBufferPush(&device->readBuffer, buffer, length);
    ASSERT(RT_SUCCESS(status), "Buffering data received from io device failed\n");
    UNREFERENCED_PARAMETER(status);
}
