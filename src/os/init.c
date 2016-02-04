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
    // The system idle task - nothing else can have this priority
    Create(IDLE_PRIORITY, IdleTask);

    // Name server MUST be created now, as its id is hard coded
    Create(NAME_SERVER_PRIORITY, NameServerTask);

    // Now the clock server
    Create(CLOCK_SERVER_PRIORITY, ClockServerTask);
}
