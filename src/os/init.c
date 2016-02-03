#include "init.h"

#include <rtos.h>

#include "name_server.h"
#include "rps.h"

VOID
InitTask
    (
        VOID
    )
{
    // Name server MUST be created first, as its id is hard coded
    Create(MEDIUM_PRIORITY, NameServerTask);

    // Start the game of rock-paper-scissors
    RpsInit();
}
