#include "stdlib.h"

// Taken from the BWIO library

int a2d( char ch ) {
    if( ch >= '0' && ch <= '9' ) return ch - '0';
    if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
    if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
    return -1;
}

char a2i( char ch, char **src, int base, int *nump ) {
    int num = 0;
    char *p = *src;
    int digit;
    while ((digit = a2d(ch)) >= 0) {
        if (digit > base)
            break;
        num = num * base + digit;
        ch = *p++;
    }
    *src = p;
    *nump = num;
    return ch;
}

void ui2a( unsigned int num, unsigned int base, char *bf ) {
    int n = 0;
    int dgt;
    unsigned int d = 1;

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
}

void i2a( int num, char *bf ) {
    if( num < 0 ) {
        num = -num;
        *bf++ = '-';
    }
    ui2a( num, 10, bf );
}

int isspace( int c ) {
    switch (c) {
        case ' ':
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
            return 1;
    }
    return 0;
}

RT_STATUS
RtAtoi
    (
        IN STRING src,
        OUT INT* num
    )
{
    INT sign = 1;

    if (*src == '-' )
    {
        sign = -1;
        src++;
    }

    INT val = 0;
    while (*src)
    {
        if ('0' <= *src && *src <= '9')
        {
            val *= 10;
            val += (*src - '0');
            src++;
        }
        else
        {
            return STATUS_FAILURE;
        }
    }

    *num = sign * val;
    return STATUS_SUCCESS;
}


RT_STATUS
RtAtoui
    (
        IN STRING src,
        OUT UINT* num
    )
{
    UINT val = 0;
    while (*src)
    {
        if ('0' <= *src && *src <= '9')
        {
            val *= 10;
            val += (*src - '0');
            src++;
        }
        else
        {
            return STATUS_FAILURE;
        }
    }

    *num = val;
    return STATUS_SUCCESS;
}
