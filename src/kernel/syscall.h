#pragma once

#include "rt.h"
#include "task.h"

VOID
SyscallInit
    (
        VOID
    );

INT
SystemCreateTask
    (
        IN INT priority,
        IN TASK_START_FUNC startFunc
    );

INT
SystemGetCurrentTaskId
    (
        VOID
    );

INT
SystemGetCurrentParentTaskId
    (
        VOID
    );

VOID
SystemPassCurrentTask
    (
        VOID
    );

VOID
SystemDestroyCurrentTask
    (
        VOID
    );

INT
SystemSendMessage
    (
        IN INT taskId,
        IN PVOID message,
        IN INT messageLength,
        IN PVOID reply,
        IN INT replyLength
    );

INT
SystemReceiveMessage
    (
        OUT INT* taskId,
        IN PVOID message,
        IN INT messageLength
    );

INT
SystemReplyMessage
    (
        IN INT taskId,
        IN PVOID reply,
        IN INT replyLength
    );
