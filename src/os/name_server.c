#include "name_server.h"

#include <rtkernel.h>
#include <rtosc/assert.h>
#include <rtosc/string.h>

#define NAME_SERVER_HASH_TABLE_SIZE (NUM_TASKS * 2)

#define ERROR_SUCCESS 0
#define ERROR_NAME_SERVER_FULL -1
#define ERROR_NAME_SERVER_NOT_FOUND -1

typedef enum _NAME_SERVER_REQUEST_TYPE
{
    RegisterRequest = 0,
    WhoIsRequest
} NAME_SERVER_REQUEST_TYPE;

typedef struct _NAME_SERVER_REQUEST
{
    NAME_SERVER_REQUEST_TYPE type;
    STRING name;
} NAME_SERVER_REQUEST;

typedef struct _NAME_SERVER_ENTRY
{
    STRING key;
    INT value;
} NAME_SERVER_ENTRY;

static INT g_nameServerId;

static
inline
VOID
NameServerpInitializeHashTable
    (
        IN NAME_SERVER_ENTRY* hashTable
    )
{
    UINT i;

    for (i = 0; i < NAME_SERVER_HASH_TABLE_SIZE; i++)
    {
        NAME_SERVER_ENTRY* entry = &hashTable[i];

        entry->key = "";
        entry->value = -1;
    }
}

static
inline
UINT
NameServerpHash
    (
        IN STRING name
    )
{
    CHAR c;
    UINT i = 0;
    UINT hash = 0;

    // Index dependant string hashing algorithm
    // Inspired by notes from CS240
    while ('\0' != (c = name[i]))
    {
        hash = (73 * hash) + c;
        i++;
    }

    return hash;
}

static
inline
BOOLEAN
NameServerpInsert
    (
        IN NAME_SERVER_ENTRY* hashTable,
        IN STRING key,
        IN INT value
    )
{
    UINT hash = NameServerpHash(key);
    UINT i;

    for (i = 0; i < NAME_SERVER_HASH_TABLE_SIZE; i++)
    {
        UINT index = (hash + i) % NAME_SERVER_HASH_TABLE_SIZE;
        NAME_SERVER_ENTRY* entry = &hashTable[index];

        if (entry->key == "" || RtStrEqual(key, entry->key))
        {
            entry->key = key;
            entry->value = value;

            return TRUE;
        }
    }

    return FALSE;
}

static
inline
BOOLEAN
NameServerpFind
    (
        IN NAME_SERVER_ENTRY* hashTable,
        IN STRING key,
        OUT INT* value
    )
{
    UINT hash = NameServerpHash(key);
    UINT i;

    for (i = 0; i < NAME_SERVER_HASH_TABLE_SIZE; i++)
    {
        UINT index = (hash + i) % NAME_SERVER_HASH_TABLE_SIZE;
        NAME_SERVER_ENTRY* entry = &hashTable[index];

        if (RtStrEqual(key, entry->key))
        {
            *value = entry->value;

            return TRUE;
        }
    }

    return FALSE;
}

VOID
NameServerpTask()
{
    NAME_SERVER_ENTRY hashTable[NAME_SERVER_HASH_TABLE_SIZE];

    NameServerpInitializeHashTable(hashTable);

    while (1)
    {
        INT senderTaskId;
        NAME_SERVER_REQUEST request;
        VERIFY(SUCCESSFUL(Receive(&senderTaskId, &request, sizeof(request))));

        INT response;
        switch(request.type)
        {
            case RegisterRequest:
            {
                BOOLEAN success = NameServerpInsert(hashTable, request.name, senderTaskId);

                response = success ? ERROR_SUCCESS : ERROR_NAME_SERVER_FULL;
                break;
            }
            case WhoIsRequest:
            {
                BOOLEAN success = NameServerpFind(hashTable, request.name, &response);

                if (!success)
                {
                    response = ERROR_NAME_SERVER_NOT_FOUND;
                    ASSERT(FALSE);
                }

                break;
            }
        }

        VERIFY(SUCCESSFUL(Reply(senderTaskId, &response, sizeof(response))));
    }
}

VOID
NameServerCreateTask()
{
    g_nameServerId = Create(Priority29, NameServerpTask);
    ASSERT(SUCCESSFUL(g_nameServerId));
}

static
inline
INT
SendNameServerRequest
    (
        IN NAME_SERVER_REQUEST_TYPE type,
        IN STRING name
    )
{
    NAME_SERVER_REQUEST request = { type,  name };
    INT response;

    VERIFY(SUCCESSFUL(Send(g_nameServerId,
                           &request,
                           sizeof(request),
                           &response,
                           sizeof(response))));

    return response;
}

INT
RegisterAs
    (
        IN STRING name
    )
{
    return SendNameServerRequest(RegisterRequest, name);
}

INT
WhoIs
    (
        IN STRING name
    )
{
    return SendNameServerRequest(WhoIsRequest, name);
}
