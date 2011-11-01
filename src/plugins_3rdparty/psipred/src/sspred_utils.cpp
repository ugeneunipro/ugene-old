#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sspred_utils.h"

void err( const char *s )
{
    fprintf(stderr, "%s\n", s);
}

void fail( const char *s )
{
    err(s);
    throw(s);  //to avoid exit
    //exit(1);
}

int aanum(int ch)
{

    static const int      aacvs[] =
    {
        999, 0, 20, 4, 3, 6, 13, 7, 8, 9, 20, 11, 10, 12, 2,
        20, 14, 5, 1, 15, 16, 20, 19, 17, 20, 18, 20
    };

    return (isalpha(ch) ? aacvs[ch & 31] : 20);
}


