#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#include <sys/mman.h>

#define KIB (1024)
#define MIB (KIB * KIB)
#define GIB (KIB * KIB * KIB)

#define MEMORY_PAGE_SIZE (KIB * 4)

struct page
{
        uint8_t bytes[MEMORY_PAGE_SIZE];
};

inline struct page* allocate_page(uint32_t count)
{
        struct page* page = (struct page*)mmap(
                NULL, MEMORY_PAGE_SIZE * count,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANON,
                -1, 0
        );
        assert(&page != MAP_FAILED);
        return page;
}

inline void destroy_page(struct page* page, uint32_t count)
{
        (void)munmap(page, MEMORY_PAGE_SIZE * count);
}

#endif
