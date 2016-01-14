#pragma once

#include "rt.h"

#define COM1_HANDLE 0
#define COM2_HANDLE 1

RT_STATUS
UartDriverInit
    (
        VOID
    );

// Temporary for A0
RT_STATUS
UartPollingUpdate
    (
        VOID
    );
