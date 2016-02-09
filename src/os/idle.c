#include "idle.h"

#include <rtos.h>
#include <rtosc/assert.h>

static volatile BOOLEAN g_running;

inline
VOID
IdleInit
    (
        VOID
    )
{
    g_running = FALSE;
}

inline
VOID
IdleExit
    (
        VOID
    )
{
    g_running = FALSE;
}

static
inline
BOOLEAN
IdlepIsRunning
    (
        VOID
    )
{
    return g_running;
}

static
VOID
IdlepTask
    (
        VOID
    )
{
    while(IdlepIsRunning()) {  }
}

static
inline
VOID
IdlepStart
    (
        VOID
    )
{
    g_running = TRUE;
}

VOID
IdleCreateTask
    (
        VOID
    )
{
    VERIFY(!IdlepIsRunning());

    IdlepStart();

    INT idleTaskId = Create(IdlePriority, IdlepTask);
    ASSERT(idleTaskId == 1);
    UNREFERENCED_PARAMETER(idleTaskId);
}
