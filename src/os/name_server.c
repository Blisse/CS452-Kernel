#include "name_server.h"

#include <rtos.h>

#include <rtosc/assert.h>
#include <rtosc/string.h>

#define NAME_SERVER_TID 2
#define NAME_SERVER_HASH_TABLE_SIZE 100

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

static
inline
VOID
NameServerpInitializeHashTable
    (
        IN NAME_SERVER_ENTRY* hashTable
    )
{
    UINT i;

    for(i = 0; i < NAME_SERVER_HASH_TABLE_SIZE; i++)
    {
        NAME_SERVER_ENTRY* entry = &hashTable[i];

        entry->key = "";
        entry->value = -1;
    }
}

static
inline
INT
NameServerpHash
    (
        IN STRING name
    )
{
    CHAR c;
    INT i = 0;
    INT hash = 0;

    // Index dependant string hashing algorithm
    // Inspired by notes from CS240
    while('\0' != (c = name[i]))
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
    INT hash = NameServerpHash(key);
    INT i;

    for(i = 0; i < NAME_SERVER_HASH_TABLE_SIZE; i++)
    {
        INT index = (hash + i) % NAME_SERVER_HASH_TABLE_SIZE;
        NAME_SERVER_ENTRY* entry = &hashTable[index];

        if(entry->key == "" || RtStrEqual(key, entry->key))
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
    INT hash = NameServerpHash(key);
    INT i;

    for(i = 0; i < NAME_SERVER_HASH_TABLE_SIZE; i++)
    {
        INT index = (hash + i) % NAME_SERVER_HASH_TABLE_SIZE;
        NAME_SERVER_ENTRY* entry = &hashTable[index];

        if(RtStrEqual(key, entry->key))
        {
            *value = entry->value;

            return TRUE;
        }
    }

    return FALSE;
}

VOID
NameServerTask
    (
        VOID
    )
{
    NAME_SERVER_ENTRY hashTable[NAME_SERVER_HASH_TABLE_SIZE];

    NameServerpInitializeHashTable(hashTable);

    while(1)
    {
        NAME_SERVER_REQUEST request;
        INT response;
        INT senderTaskId;
        BOOLEAN success;

        Receive(&senderTaskId, &request, sizeof(request));

        switch(request.type)
        {
            case RegisterRequest:
                success = NameServerpInsert(hashTable,
                                            request.name,
                                            senderTaskId);

                response = success ? ERROR_SUCCESS : ERROR_NAME_SERVER_FULL;
                break;

            case WhoIsRequest:
                success = NameServerpFind(hashTable,
                                          request.name,
                                          &response);

                if(!success)
                {
                    response = ERROR_NAME_SERVER_NOT_FOUND;
                    ASSERT(FALSE, "Received WhoIs request for unknown name \r\n");
                }

                break;

            default:
                ASSERT(FALSE, "Name server received unknown request \r\n");
        }

        Reply(senderTaskId, &response, sizeof(response));
    }
}

static
inline
INT
SendNameServerRequest
    (
        NAME_SERVER_REQUEST_TYPE type,
        STRING name
    )
{
    NAME_SERVER_REQUEST request = { type,  name };
    INT response;

    Send(NAME_SERVER_TID,
         &request,
         sizeof(request),
         &response,
         sizeof(response));

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
