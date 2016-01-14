#include "object.h"
#include "assert.h"

#define HANDLE_TABLE_SIZE 2

static PVOID handleTable[HANDLE_TABLE_SIZE];

static
inline
BOOLEAN
ObjectpCheckHandle
    (
        IN HANDLE handle
    )
{
    return 0 <= handle && handle < HANDLE_TABLE_SIZE;
}

VOID
ObjectRegisterHandle
    (
        IN HANDLE handle,
        IN PVOID object
    )
{
    ASSERT(ObjectpCheckHandle(handle), "Invalid handle passed to ObjectRegisterHandle\n");

    handleTable[handle] = object;
}

PVOID
ObjectGetFromHandle
    (
        IN HANDLE handle
    )
{
    ASSERT(ObjectpCheckHandle(handle), "Invalid handle passed to ObjectGetFromHandle\n");

    return handleTable[handle];
}
