#include "string.h"

#include "assert.h"
#include "stdlib.h"

// TODO: Optimize all of these functions
// NOTE: Much of this code is copied from code I wrote in SE350.
//       I'm pretty sure that's OK since I'm copying code that
//       I previously wrote, but just figured I'd put a note.

INT
RtStrCmp
    (
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

UINT
RtStrLen
    (
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

INT
RtAtoi
    (
        STRING str
    )
{
    INT result = 0;
    INT sign = 1;
    STRING iter = str;

    //Skip all whitespace
    while(' ' == *iter)
    {
        iter++;
    }

    //Check for sign bit
    if('+' == *iter || '-' == *iter)
    {
        //Check if sign is negative
        if('-' == *iter)
        {
            sign = -1;
        }

        iter++;
    }

    //While we're still looking at a number
    while(*iter >= '0' && *iter <= '9')
    {
        result *= 10;
        result += *iter - '0';
        iter++;
    }

    return sign * result;
}

CHAR*
RtStrFind
    (
        STRING str,
        STRING substr
    )
{
    STRING pos = NULL;

    if(*substr != '\0')
    {
        while(*str != '\0')
        {
            if(*str == *substr)
            {
                STRING posInString = str + 1;
                STRING posInSubstring = substr + 1;

                while(*posInString != '\0' &&
                      *posInSubstring != '\0' &&
                      *posInString++ == *posInSubstring++)
                {
                }

                if(*posInSubstring == '\0')
                {
                    pos = str;
                    break;
                }
            }

            str++;
        }
    }

    return pos;
}

typedef struct _RT_STR_FORMAT_PARAMS
{
    CHAR lz;
    CHAR sign;
    INT width;
    UINT base;
    STRING bf;
} RT_STR_FORMAT_PARAMS;

typedef struct _RT_STR_FORMAT_OUTPUT
{
    STRING destination;
    INT capacity;
    INT length;
} RT_STR_FORMAT_OUTPUT;

VOID
RtStrpFormatParamsInit
    (
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
RtStrpFormatOutputInit
    (
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
RtStrpPutChar
    (
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
RtStrpPutBf
    (
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
RtStrpPrintFormatted
    (
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
RtStrPrintFormatted
    (
        OUT STRING ret,
        IN INT retLength,
        IN STRING fmt,
        ...
    )
{
    if (retLength < 1)
    {
        return 0;
    }

    RT_STR_FORMAT_OUTPUT output;
    RtStrpFormatOutputInit(&output, ret, retLength - 1);

    VA_LIST va;
    VA_START(va, fmt);
    RtStrpPrintFormatted(fmt, &output, va);
    VA_END(va);

    output.destination[min(output.length, output.capacity)] = '\0';

    return output.length;
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
