#pragma once

#include <rt.h>

int a2d( char ch );

char a2i( char ch, char **src, int base, int *nump );

void ui2a( unsigned int num, unsigned int base, char *bf );

void i2a( int num, char *bf );

int isspace( int c );

RT_STATUS
RtAtoi
    (
        IN STRING src,
        OUT INT* num
    );
