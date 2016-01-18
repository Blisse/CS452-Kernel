#include "rt.h"
#include "bwio/bwio.h"
#include "swi.h"

VOID
FirstUserTask
    (
        VOID
    );

extern int Test();
extern int GetCpsr();

INT
main
    (
        VOID
    ) 
{
    int blah;
    int cpsr;

    bwsetfifo(BWCOM2, OFF);
    bwsetspeed(BWCOM2, 115200);

    cpsr = GetCpsr();

    bwprintf(BWCOM2, "CPSR %d \r\n", cpsr);

    bwprintf(BWCOM2, "Installing swi handler\r\n");

    InstallSwiHandler();

    bwprintf(BWCOM2, "Making a system call\r\n");

    blah = Test();

    bwprintf(BWCOM2, "Testing %d \r\n", blah);

    return STATUS_SUCCESS;
}

VOID
print(VOID)
{
    bwprintf(BWCOM2, "Blahhhhhhhh \r\n");
}
