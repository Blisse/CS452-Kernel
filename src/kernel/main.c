#include "rt.h"
#include "kernel.h"

INT
main
    (
        VOID
    )
{
    KernelInit();
    KernelRun();
    return STATUS_SUCCESS;
}
