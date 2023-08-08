#ifndef MEMORY_H
#define MEMORY_H

#include <immintrin.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

#define KIB (1024)
#define MIB (KIB * KIB)
#define GIB (KIB * KIB * KIB)

#define MEMORY_PAGE_SIZE (KIB * 4)

__attribute__((always_inline))
inline void* allocate_memory(size_t size)
{
        return malloc(size);
}

__attribute__((always_inline))
inline void free_memory(void* pointer)
{
        free(pointer);
}

__attribute__((always_inline))
inline void* allocate_memory_page(size_t count)
{
         return mmap(NULL, MEMORY_PAGE_SIZE * count,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON,
                     -1, 0);
}

__attribute__((always_inline))
inline int free_memory_page(void*    page,
                            uint32_t count)
{
        return munmap(page, MEMORY_PAGE_SIZE * count);
}

#define prefetch_memory_t0(p) _mm_prefetch(p, _MM_HINT_T0);

#endif
