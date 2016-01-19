#include "rtos.h"
#include <bwio/bwio.h>

extern
INT
GetCPSR
    (
        VOID
    );

extern
UINT*
GetSP
    (
        VOID
    );

VOID
InitTask
    (
        VOID
    )
{
    bwprintf(BWCOM2, "User CPSR is: %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "User SP is: %d \r\n", GetSP());
    Pass();

    bwprintf(BWCOM2, "User CPSR is: %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "User SP is: %d \r\n", GetSP());
    Pass();

    bwprintf(BWCOM2, "User CPSR is: %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "User SP is: %d \r\n", GetSP());
    Pass();

    bwprintf(BWCOM2, "User CPSR is: %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "User SP is: %d \r\n", GetSP());
    Pass();
}
