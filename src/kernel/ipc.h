#pragma once

#include "rt.h"
#include "task.h"

inline
VOID
IpcInitializeMailbox
    (
        IN TASK_DESCRIPTOR* td
    );

RT_STATUS
IpcSend
    (
        IN TASK_DESCRIPTOR* from, 
        IN TASK_DESCRIPTOR* to, 
        IN PVOID message, 
        IN INT messageLength, 
        IN PVOID replyBuffer, 
        IN INT replyBufferLength
    );

RT_STATUS
IpcReceive
    (
        IN TASK_DESCRIPTOR* td, 
        IN INT* senderId,
        IN PVOID buffer, 
        IN INT bufferLength, 
        OUT INT* bytesReceived
    );

RT_STATUS
IpcReply
    (
        IN TASK_DESCRIPTOR* from, 
        IN TASK_DESCRIPTOR* to, 
        IN PVOID reply, 
        IN INT replyLength
    );
