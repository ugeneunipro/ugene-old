#ifndef _UHMM3_FUNCS_H_
#define _UHMM3_FUNCS_H_

#ifdef _WINDOWS

#define snprintf    sprintf_s

extern int      isnan( float x );
extern int      isinf( float x );
extern float    roundf( float x );

#else // if not Windows -> we have all functions and defines in math.h
#include <math.h>
#endif // _WINDOWS

extern float infinity();
extern bool isfin( float x ); // sure that number is finite

#define EXCEPTION_MSG_SZ 512
struct UHMMER3Exception {
    char msg[EXCEPTION_MSG_SZ];
    
}; // UHMMER3Exception

extern void throwUHMMER3Exception( const char* str );

#endif // _UHMM3_FUNCS_H_
