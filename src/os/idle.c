#include "idle.h"

#include <rtos.h>
#include <rtosc/assert.h>

static volatile BOOLEAN g_running;

VOID
Shutdown
    (
        VOID
    )
{
    g_running = FALSE;
}

static
VOID
IdlepTask
    (
        VOID
    )
{
    g_running = TRUE;

    while(g_running) {  }
}

VOID
IdleCreateTask
    (
        VOID
    )
{
    INT idleTaskId = Create(IdlePriority, IdlepTask);
    ASSERT(idleTaskId == 1);
    UNREFERENCED_PARAMETER(idleTaskId);
}
