#include "rt.h"
#include "arm.h"
#include "bwio/bwio.h"
#include "kernel.h"
#include "trap.h"

extern
VOID
InitTask
    (
        VOID
    );

static UINT testStack[256];

typedef struct _TASK {
    UINT* stack;
    PVOID retval;
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

    *(testStack - 10) = (UINT) InitTask;
    *(testStack - 11) = 0x10;
    *(testStack - 12) = 0x30;
    testStack -= 12;

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

    bwprintf(BWCOM2, "Initializing kernel \r\n");
    KernelInit();

    bwprintf(BWCOM2, "Creating test task \r\n");

    TestCreate(&test, testStack, 256);

    bwprintf(BWCOM2, "Init task addr is %d \r\n", (UINT) InitTask);
    bwprintf(BWCOM2, "Starting CPSR is %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "Top of stack is %d \r\n", (UINT)(testStack + 255));
    bwprintf(BWCOM2, "SP should be %d \r\n", (UINT) test.stack);

    TrapReturn(test.stack);
    test.stack = GetUserSP();

    bwprintf(BWCOM2, "Kernel CPSR is %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "Kernel has user SP as %d \r\n", (UINT) test.stack);

    TrapReturn(test.stack);
    test.stack = GetUserSP();

    bwprintf(BWCOM2, "Kernel CPSR is %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "Kernel has user SP as %d \r\n", (UINT) test.stack);

    TrapReturn(test.stack);
    test.stack = GetUserSP();

    bwprintf(BWCOM2, "Kernel CPSR is %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "Kernel has user SP as %d \r\n", (UINT) test.stack);

    TrapReturn(test.stack);
    test.stack = GetUserSP();

    bwprintf(BWCOM2, "Kernel CPSR is %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "Kernel has user SP as %d \r\n", (UINT) test.stack);

    TrapReturn(test.stack);
    test.stack = GetUserSP();

    bwprintf(BWCOM2, "Kernel CPSR is %d \r\n", GetCPSR());
    bwprintf(BWCOM2, "Kernel has user SP as %d \r\n", (UINT) test.stack);

    return STATUS_SUCCESS;
}

VOID
print
    (
        VOID
    )
{
    bwprintf(BWCOM2, "R8 is %d \r\n", GetR8());
}
