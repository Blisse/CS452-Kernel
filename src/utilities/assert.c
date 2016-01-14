#include "assert.h"

#include <bwio/bwio.h>

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

    bwsetfifo(BWCOM2, OFF);
    bwprintf(BWCOM2, "Assert triggered on line %d of file %s\n", line, file);
    bwputstr(BWCOM2, msg);

    while(1) { }
}
