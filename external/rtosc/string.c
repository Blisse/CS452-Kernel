#include "string.h"

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

// TODO: Optimize this with NEON or Load-Multiple
static
inline
VOID
memcpy_unaligned
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
memcpy_aligned
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

    memcpy_unaligned(d, s, bytes % sizeof(INT));
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
        memcpy_aligned(dest, src, bytes);
    }
    else
    {
        memcpy_unaligned(dest, src, bytes);
    }
}

// These functions were taken from bwio
// All credit goes to the authors of bwio
void RtUitoa( unsigned int num, char *bf, unsigned int* len ) {
    int n = 0;
    int dgt;
    unsigned int d = 1;
    unsigned int base = 10;
    char* orig = bf;

    while( (num / d) >= base ) d *= base;
    while( d != 0 ) {
        dgt = num / d;
        num %= d;
        d /= base;
        if( n || dgt > 0 || d == 0 ) {
            *bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
            ++n;
        }
    }
    *bf = 0;
    *len = bf - orig;
}
