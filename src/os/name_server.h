#pragma once

#include "rt.h"
#include "rtos.h"

VOID
NameServerCreateTask
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
