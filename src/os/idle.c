#include "idle.h"

#include <bwio/bwio.h>

VOID
IdleTask
    (
        VOID
    )
{
    // TODO - Measure system performance
    while(1) 
    {
        bwprintf(BWCOM2, "IDLE\r\n");
    }
}
