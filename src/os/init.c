#include "rtos.h"
#include <bwio/bwio.h>

extern
INT
GetSP
    (
        VOID
    );

VOID
FirstUserTask
    (
        VOID
    )
{
    // bwprintf(BWCOM2, "IN FIRST USER TASK \r\n");
    // bwprintf(BWCOM2, "IN user task sp is: %d \r\n", GetSP());
    // TODO: VERIFY THAT CPSR IS IN USER MODE
    Pass();

    // bwprintf(BWCOM2, "IN FIRST USER TASK \r\n");
    // bwprintf(BWCOM2, "IN user task sp is: %d \r\n", GetSP());
    // TODO: VERIFY THAT CPSR IS IN USER MODE
    Pass();
}
