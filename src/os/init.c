#include "init.h"

#include <bwio/bwio.h>
#include "nameserver.h"
#include <rtos.h>

#define RPS_SERVER_NAME "RpsServer"

static
VOID
RpsClientTask
    (
        VOID
    )
{
    INT rpsServerTaskId = WhoIs(RPS_SERVER_NAME);
    INT message = MyTid();
    INT reply;

    bwprintf(BWCOM2, "Client sending %d to server \r\n", message);

    Send(rpsServerTaskId,
         &message, 
         sizeof(message),
         &reply, 
         sizeof(reply));

    bwprintf(BWCOM2, "Client received %d from server \r\n", reply);
}

static
VOID
RpsServerHandleClient
    (
        VOID
    )
{
    INT message;
    INT sender;

    Receive(&sender, &message, sizeof(message));

    bwprintf(BWCOM2, "Server received %d from client \r\n", message);

    Reply(sender, &message, sizeof(message));
}

static
VOID
RpsServerTask
    (
        VOID
    )
{
    RegisterAs(RPS_SERVER_NAME);
    RpsServerHandleClient();
    RpsServerHandleClient();
}

VOID
InitTask
    (
        VOID
    )
{
    // Name server MUST be created first, as its id is hard coded
    Create(MEDIUM_PRIORITY, NameServerTask);

    // Start the game of rock-paper-scissors
    Create(MEDIUM_PRIORITY, RpsServerTask);
    Create(LOW_PRIORITY, RpsClientTask);
    Create(LOW_PRIORITY, RpsClientTask);
}
