#include <stdio.h>
#include <string.h>
#include <limits>

#include <QtCore/QtGlobal>

#include "hmmer3_funcs.h"

#ifdef _WINDOWS

int isnan( float x ) {
    return x != x;
}

int isinf( float x ) {
    float inf = infinity();
    return inf == x || inf == -x;
}

float roundf( float x ){
    if( isnan( x ) || isinf( x ) ) {
        return x;
    }
    return (float)(x >= 0.0 ? (int)(x + 0.5) : (int)(x - (int)(x-1) + 0.5) + (int)(x-1));
}

#endif // _WINDOWS

float infinity() {
    return std::numeric_limits< float >::infinity();
}

bool isfin( float x ) {
    return !isnan( x ) && !isinf( x );
}

const char TERM_SYM = '\0';

void throwUHMMER3Exception( const char* str ) {
    UHMMER3Exception ex;
    int strSz = qMin( (int)strlen( str ), EXCEPTION_MSG_SZ - 1 );
    strncpy( ex.msg, str, strSz );
    ex.msg[strSz] = TERM_SYM;
    throw ex;
}
