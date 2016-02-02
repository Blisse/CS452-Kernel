#include "init.h"

#include <bwio/bwio.h>
#include "nameserver.h"
#include <rtosc/assert.h>

#include "name_server.h"
#include "rps.h"

VOID
InitTask
    (
        VOID
    )
{
    // Name server MUST be created first, as its id is hard coded
    Create(NAME_SERVER_PRIORITY, NameServerTask);

    // Start the game of rock-paper-scissors
    RpsInit();
}
