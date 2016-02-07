#include "idle.h"

#include <bwio/bwio.h>
#include <rtos.h>
#include <rtosc/assert.h>

static BOOLEAN g_running;

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
    while(IdlepIsRunning())
    {
        bwprintf(BWCOM2, "IDLE\r\n");
    }
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
    VERIFY(!IdlepIsRunning(), "Only one idle task may exist at any time.");

    IdlepStart();

    INT idleTaskId = Create(IdlePriority, IdlepTask);
    ASSERT(idleTaskId == 1, "Idle task must be first created task.");
    UNREFERENCED_PARAMETER(idleTaskId);
}
