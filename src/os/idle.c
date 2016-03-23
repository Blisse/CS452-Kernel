#include "idle.h"

#include <rtkernel.h>
#include <rtosc/assert.h>

static volatile BOOLEAN g_running;

static
VOID
IdlepTask()
{
    while (g_running) {  }
}

VOID
IdleCreateTask()
{
    g_running = TRUE;

    VERIFY(SUCCESSFUL(Create(IdlePriority, IdlepTask)));
}

VOID
IdleQuit()
{
    g_running = FALSE;
}
