#pragma once

#include "rt.h"

typedef INT HANDLE;

VOID
ObjectRegisterHandle
    (
        IN HANDLE handle,
        IN PVOID object
    );

PVOID
ObjectGetFromHandle
    (
        IN HANDLE handle
    );
