#include "init.h"

#include "rtos.h"
#include <bwio/bwio.h>

VOID
InitTask
    (
        VOID
    )
{
    bwprintf(BWCOM2, "Init Task Running...\r\n");
    bwprintf(BWCOM2, "FirstUserTask: exiting\r\n");

    Exit();
}
