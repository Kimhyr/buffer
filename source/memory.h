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

struct memory_page
{
        uint8_t bytes[MEMORY_PAGE_SIZE];
};

inline struct memory_page* allocate_page(
        uint32_t count)
{
         return (struct memory_page*)mmap(
                NULL, MEMORY_PAGE_SIZE * count,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANON,
                -1, 0);
}

inline int destroy_page(
        struct memory_page* page,
        uint32_t            count)
{
        return munmap(page, MEMORY_PAGE_SIZE * count);
}

#define prefetch_memory_t0(p) _mm_prefetch(p, _MM_HINT_T0);

#endif
