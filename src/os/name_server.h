#pragma once

#include "rt.h"
#include "rtos.h"

#define NAME_SERVER_PRIORITY PRIORITY_28

VOID
NameServerTask
    (
        VOID
    );

INT
RegisterAs
    (
        IN STRING name
    );

INT
WhoIs
    (
        IN STRING name
    );
