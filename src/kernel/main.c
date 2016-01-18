#include "rt.h"
#include "bwio/bwio.h"
#include "trap.h"

VOID
FirstUserTask
    (
        VOID
    );

extern int GetCpsr();

INT
main
    (
        VOID
    ) 
{
    int cpsr;

    bwsetfifo(BWCOM2, OFF);
    bwsetspeed(BWCOM2, 115200);

    cpsr = GetCpsr();

    bwprintf(BWCOM2, "CPSR %d \r\n", cpsr);
    bwprintf(BWCOM2, "Installing swi handler\r\n");

    TrapInstallHandler();

    return STATUS_SUCCESS;
}
