#ifndef SIMD_H
#define SIMD_H

#include <cstddef>

namespace simd
{

template<typename T>
T* memcpy(T* __restrict source,
          T* __restrict destination,
          std::size_t   count);

template<typename T, typename U>
T* memset(T*          source,
          U           replacement,
          std::size_t count);

}

#endif
