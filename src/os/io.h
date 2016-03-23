#pragma once

#include <rt.h>
#include <rtos.h>
#include <rtkernel.h>

typedef
INT
(*IO_OPEN_FUNC)
    (
        IN IO_CHANNEL channel,
        OUT IO_DEVICE* device
    );

typedef
CHAR
(*IO_READ_FUNC)();

typedef
VOID
(*IO_WRITE_FUNC)
    (
        CHAR c
    );

VOID
IoCreateTask();

INT
IoRegisterDriver
    (
        IN IO_DEVICE_TYPE type,
        IN IO_OPEN_FUNC openFunc
    );

INT
IoCreateReadTask
    (
        IN TASK_PRIORITY priority,
        IN EVENT event,
        IN IO_READ_FUNC readFunc,
        IN STRING name
    );

INT
IoCreateWriteTask
    (
        IN TASK_PRIORITY priority,
        IN EVENT event,
        IN IO_WRITE_FUNC writeFunc,
        IN STRING name
    );
