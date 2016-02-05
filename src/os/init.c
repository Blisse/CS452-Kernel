#include "init.h"

#include "idle.h"
#include "clock_server.h"
#include "name_server.h"
#include <rtosc/assert.h>

VOID
InitTask
    (
        VOID
    )
{
    IdleCreateTask();

    NameServerCreateTask();

    ClockServerCreateTask();
}
