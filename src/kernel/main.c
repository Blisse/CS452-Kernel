#include <rt.h>

#include <bwio/bwio.h>
#include "kernel.h"

INT
main()
{
    // Setup bwio so that asserts can work
    bwsetfifo(BWCOM2, OFF);
    bwsetspeed(BWCOM2, 115200);
    bwprintf(BWCOM2, "\r\n");

    // Run the kernel
    KernelRun();

    return STATUS_SUCCESS;
}
