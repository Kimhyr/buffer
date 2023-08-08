#ifndef SIMD_H
#define SIMD_H

#include <memory.h>

inline void *simd_memcpy(void*  d,
                         void*  s,
                         size_t n)
{ return memcpy(d, s, n); }

inline void *simd_memset(void*  s,
                         char   c,
                         size_t n)
{ return memset(s, c, n);}

#endif
