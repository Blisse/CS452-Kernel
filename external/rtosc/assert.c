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
        STRING msg,
        INT line,
        STRING file
    )
{
    if(expr)
    {
        return;
    }

#if NLOCAL
    bwprintf(BWCOM2, "Assert triggered on line %d of file %s \r\n", line, file);
    bwputstr(BWCOM2, msg);
    bwputstr(BWCOM2, "\r\n");
#else
    printf("Assert triggered on line %d of file %s \r\n %s \r\n", line, file, msg);
#endif

    while(1) { }
}
