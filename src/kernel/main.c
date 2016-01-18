#include "rt.h"
#include "bwio/bwio.h"
#include "trap.h"

VOID
FirstUserTask
    (
        VOID
    );

VOID
Pass
    (
        VOID
    );

extern int GetCpsr();

static UINT testStack[256];

typedef struct _TASK {
    UINT* stack;
    int retval;
} TASK;

VOID
TestCreate
    (
        IN TASK* task,
        IN UINT* stack, 
        IN UINT stackSize
    )
{
    UINT i;
    UINT* testStack = stack + stackSize - 1;

    for(i = 0; i < stackSize; i++)
    {
        stack[i] = 0x20;
    }

    *(testStack - 10) = (UINT) FirstUserTask;
    *(testStack - 11) = 0x10;
    testStack -= 11;

    task->stack = testStack;
    task->retval = 0;
}

INT
main
    (
        VOID
    ) 
{
    TASK test;

    bwsetfifo(BWCOM2, OFF);
    bwsetspeed(BWCOM2, 115200);

    bwprintf(BWCOM2, "Installing swi handler\r\n");
    TrapInstallHandler();

    // bwprintf(BWCOM2, "Creating test task \r\n");

    TestCreate(&test, testStack, 256);

    // bwprintf(BWCOM2, "R3 should be %d \r\n", (UINT) Pass);
    // bwprintf(BWCOM2, "Top of stack is %d \r\n", (UINT)(testStack + 255));
    // bwprintf(BWCOM2, "SP should be %d \r\n", (UINT) test.stack);
    // bwprintf(BWCOM2, "Calling test task \r\n");

    test.retval = TrapReturn(test.retval, test.stack);
    test.stack = GetUserSP();

    // bwprintf(BWCOM2, "Ending with value %d\r\n", test.retval);
    // bwprintf(BWCOM2, "SP is %d \r\n", (UINT) test.stack);

    test.retval = TrapReturn(test.retval, test.stack);
    test.stack = GetUserSP();

    // bwprintf(BWCOM2, "Ending with value %d\r\n", test.retval);
    // bwprintf(BWCOM2, "SP is %d \r\n", (UINT) test.stack);

    return STATUS_SUCCESS;
}

// VOID
// print
//     (
//         VOID
//     )
// {
//     bwprintf(BWCOM2, "R1 is %d \r\n", GetR3());
//     bwprintf(BWCOM2, "SPSR is %d \r\n", GetSPSR());
//     bwprintf(BWCOM2, "SP is %d \r\n", GetSP());
// }
