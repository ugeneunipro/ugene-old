#ifndef _U64_H_
#define _U64_H_

#ifdef _MSC_VER
#define U64(x) x##i64
#else
#define U64(x) x##llu
#endif

#endif
