#include "simd.h"

#include <memory.h>

void* simd_memcpy(void*  source,
                  void*  destination,
                  size_t count)
{
    return memcpy(destination, source, count);
}

void* simd_memset(void*  source,
                  char   value,
                  size_t count)
{
    return memset(source, value, count);
}
