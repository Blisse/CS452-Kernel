#include "string.h"

#include "assert.h"
#include "stdlib.h"

// TODO: Optimize all of these functions
// NOTE: Much of this code is copied from code I wrote in SE350.
//       I'm pretty sure that's OK since I'm copying code that
//       I previously wrote, but just figured I'd put a note.

INT
RtStrCmp (
        STRING str1,
        STRING str2
    )
{
    STRING p1 = str1;
    STRING p2 = str2;

    while('\0' != *p1)
    {
        if('\0' == *p2)
        {
            return 1;
        }
        else if(*p1 < *p2)
        {
            return -1;
        }
        else if(*p1 > *p2)
        {
            return 1;
        }

        p1++;
        p2++;
    }

    if('\0' != *p2)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

BOOLEAN
RtStrEqual (
        STRING str1,
        STRING str2
    )
{
    return RtStrCmp(str1, str2) == 0;
}

UINT
RtStrLen (
        STRING str
    )
{
    STRING iter = str;

    while(*iter != '\0')
    {
        iter++;
    }

    return iter - str;
}

typedef struct _RT_STR_FORMAT_PARAMS {
    CHAR lz;
    CHAR sign;
    INT width;
    UINT base;
    STRING bf;
} RT_STR_FORMAT_PARAMS;

typedef struct _RT_STR_FORMAT_OUTPUT {
    STRING destination;
    INT capacity;
    INT length;
} RT_STR_FORMAT_OUTPUT;

VOID
RtStrpFormatParamsInit (
        RT_STR_FORMAT_PARAMS* params,
        STRING bf
    )
{
    params->lz = ' ';
    params->sign = 0;
    params->width = 0;
    params->base = 0;
    params->bf = bf;
}

VOID
RtStrpFormatOutputInit (
        RT_STR_FORMAT_OUTPUT* output,
        STRING destination,
        INT capacity
    )
{
    output->destination = destination;
    output->capacity = capacity;
    output->length = 0;
}

static
inline
VOID
RtStrpPutChar (
        RT_STR_FORMAT_OUTPUT* output,
        CHAR c
    )
{
    if (output->length < output->capacity)
    {
        output->destination[output->length] = c;
    }
    output->length += 1;
}

static
inline
VOID
RtStrpPutBf (
        RT_STR_FORMAT_OUTPUT* output,
        RT_STR_FORMAT_PARAMS* params
    )
{
    CHAR* p;
    INT n = params->width;

    p = params->bf;

    // Correct for padding
    while (*p++ && n > 0)
    {
        n--;
    }

    // Put sign
    if (params->sign)
    {
        n--;
        RtStrpPutChar(output, params->sign);
    }

    // Put padding
    if (params->lz)
    {
        while (n-- > 0)
        {
            RtStrpPutChar(output, params->lz);
        }
    }

    p = params->bf;

    // Put string
    CHAR c;
    while ((c = *(p++)))
    {
        RtStrpPutChar(output, c);
    }
}

static
VOID
RtStrpPrintFormattedOutput (
        IN STRING fmt,
        RT_STR_FORMAT_OUTPUT* output,
        OPTIONAL IN VA_LIST va
    )
{
    char bf[12];

    RT_STR_FORMAT_PARAMS params;

    CHAR c;
    while ((c = *(fmt++)))
    {
        if (c != '%')
        {
            RtStrpPutChar(output, c);
        }
        else
        {
            RtStrpFormatParamsInit(&params, bf);

            c = *(fmt++);
            if (c == '0')
            {
                params.lz = '0';
                c = *(fmt++);
            }

            if (c >= '0' && c <= '9')
            {
                c = a2i(c, &fmt, 10, &params.width);
            }

            switch (c) {
                case 'c':
                    RtStrpPutChar(output, VA_ARG(va, CHAR));
                    break;
                case 's':
                    params.lz = 0;
                    params.bf = VA_ARG(va, STRING);
                    RtStrpPutBf(output, &params);
                    break;
                case 'u':
                    params.base = 10;
                    ui2a(VA_ARG(va, UINT), params.base, params.bf);
                    RtStrpPutBf(output, &params);
                    break;
                case 'd':
                    params.base = 10;
                    i2a(VA_ARG(va, INT), params.bf);
                    RtStrpPutBf(output, &params);
                    break;
                case 'x':
                    params.base = 16;
                    ui2a(VA_ARG(va, UINT), params.base, params.bf);
                    RtStrpPutBf(output, &params);
                    break;
                case '%':
                    RtStrpPutChar(output, '%');
                    break;
            }
        }
    }
}

INT
RtStrpPrintFormatted (
        OUT STRING ret,
        IN INT retLength,
        IN STRING fmt,
        IN VA_LIST va
    )
{
    if (retLength < 1)
    {
        return 0;
    }

    RT_STR_FORMAT_OUTPUT output;
    RtStrpFormatOutputInit(&output, ret, retLength - 1);

    RtStrpPrintFormattedOutput(fmt, &output, va);

    output.destination[min(output.length, output.capacity)] = '\0';

    return output.length;
}

INT
RtStrPrintFormattedVa (
        OUT STRING ret,
        IN INT retLength,
        IN STRING fmt,
        IN VA_LIST va
    )
{
    return RtStrpPrintFormatted(ret, retLength, fmt, va);
}

INT
RtStrPrintFormatted (
        OUT STRING ret,
        IN INT retLength,
        IN STRING fmt,
        ...
    )
{
    INT status;

    VA_LIST va;
    VA_START(va, fmt);
    status = RtStrpPrintFormatted(ret, retLength, fmt, va);
    VA_END(va);

    return status;
}

INT
RtStrConsumeToken (
        IN CHAR** str,
        OUT CHAR* buffer,
        IN INT bufferLength
    )
{
    CHAR* p = *str;
    CHAR c;
    // ignore leading space
    while ((c = *p++) && isspace(c))
    {
    }

    // read until space
    UINT i = 0;
    while (c != '\0' && !isspace(c) && (i < bufferLength))
    {
        buffer[i++] = c;
        c = *p++;
    }

    // terminate buffer
    buffer[i] = '\0';

    // consume token
    *str = p;

    return i;
}

BOOLEAN
RtStrIsWhitespace (
        IN STRING str
    )
{
    CHAR c;
    while ((c = *str++))
    {
        if (!isspace(c))
        {
            return FALSE;
        }
    }
    return TRUE;
}

INT
RtStrpScanFormatted (
        IN STRING parse,
        IN STRING fmt,
        OPTIONAL IN VA_LIST va
    )
{
    INT status = 0;

    CHAR c;
    while ((c = *(fmt++)))
    {
        CHAR token[80];
        RtMemset(token, sizeof(token), 0);

        INT read = 0;

        if (c == '%')
        {
            c = *(fmt++);

            read = RtStrConsumeToken(&parse, token, sizeof(token));

            if (read <= 0)
            {
                return -1;
            }

            switch (c) {
                case 'c':
                {
                    if (read == 1)
                    {
                        CHAR* c = VA_ARG(va, CHAR*);
                        *c = token[0];
                    }
                    else
                    {
                        return -1;
                    }
                    break;
                }
                case 's':
                {
                    CHAR* s = VA_ARG(va, CHAR*);
                    for (UINT i = 0; i < read; i++)
                    {
                        *(s++) = token[i];
                    }
                    break;
                }
                case 'u':
                {
                    UINT* n = VA_ARG(va, UINT*);
                    VERIFY(RT_SUCCESS(RtAtoui(token, n)));
                    break;
                }
                case 'd':
                {
                    INT* n = VA_ARG(va, INT*);
                    VERIFY(RT_SUCCESS(RtAtoi(token, n)));
                    break;
                }
            }
        }

        status += read;
    }

    return status;
}

INT
RtStrScanFormatted (
        IN STRING parse,
        IN STRING fmt,
        ...
    )
{
    VA_LIST va;
    VA_START(va, fmt);
    INT status = RtStrpScanFormatted(parse, fmt, va);
    VA_END(va);

    return status;
}

static
inline
VOID
RtMemcpypUnaligned
    (
        PVOID dest,
        PVOID src,
        UINT bytes
    )
{
    CHAR* d = dest;
    CHAR* s = src;

    while(bytes > 0)
    {
        *(d++) = *(s++);
        bytes--;
    }
}

static
inline
VOID
RtMemcpypAligned
    (
        PVOID dest,
        PVOID src,
        UINT bytes
    )
{
    INT* d = dest;
    INT* s = src;
    UINT words = bytes / sizeof(INT);

    while(words > 0)
    {
        *(d++) = *(s++);
        words--;
    }

    RtMemcpypUnaligned(d, s, bytes % sizeof(INT));
}

VOID
RtMemcpy
    (
        PVOID dest,
        PVOID src,
        UINT bytes
    )
{
    if(0 == ((UINT) dest) % sizeof(UINT) &&
       0 == ((UINT) src) % sizeof(UINT))
    {
        RtMemcpypAligned(dest, src, bytes);
    }
    else
    {
        RtMemcpypUnaligned(dest, src, bytes);
    }
}

VOID
RtMemset
    (
        PVOID dest,
        UINT size,
        CHAR value
    )
{
    CHAR* p = dest;

    for (UINT i = 0; i < size; i++)
    {
        p[i] = value;
    }
}
