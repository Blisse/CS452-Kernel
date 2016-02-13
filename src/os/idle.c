#include "idle.h"

#include <rtos.h>
#include <rtosc/assert.h>

static volatile BOOLEAN g_running;

static
VOID
IdlepTask
    (
        VOID
    )
{
    while(g_running) {  }
}

VOID
IdleCreateTask
    (
        VOID
    )
{
    g_running = TRUE;

    VERIFY(SUCCESSFUL(Create(IdlePriority, IdlepTask)));
}

VOID
IdleQuit
    (
        VOID
    )
{
    g_running = FALSE;
}
