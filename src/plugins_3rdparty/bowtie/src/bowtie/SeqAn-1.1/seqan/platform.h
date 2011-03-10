#ifdef _MSC_VER
#define PLATFORM "windows"

#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS
#endif

#pragma warning( disable : 4675 )
#pragma warning( disable : 4503 )

#define finline __forceinline

//define SEQAN_SWITCH_USE_FORWARDS to use generated forwards 
#define SEQAN_SWITCH_USE_FORWARDS

#else 

#define PLATFORM "gcc"

#ifndef PLATFORM_GCC
#define PLATFORM_GCC
#endif

// should be set before including anything
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif	

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

//#include <unistd.h>
#include <inttypes.h>

#define finline __inline__

// default 64bit type
typedef int64_t __int64;


//define SEQAN_SWITCH_USE_FORWARDS to use generated forwards 
#define SEQAN_SWITCH_USE_FORWARDS

#endif // _MSC_VER