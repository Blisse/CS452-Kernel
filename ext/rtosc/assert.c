#include "assert.h"

#if NLOCAL
    #include <bwio/bwio.h>
#else
    #include <stdio.h>
#endif

VOID
assert
    (
        BOOLEAN expr,
        INT line,
        STRING file
    )
{
    if(expr)
    {
        return;
    }

#if NLOCAL
    bwprintf(BWCOM2, "Assert triggered on line %d of file %s\r\n", line, file);
#else
    printf("Assert triggered on line %d of file %s\r\n", line, file);
#endif

    while(1) { }
}
