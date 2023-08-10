#ifndef MEMORY_H
#define MEMORY_H

#include <sys/mman.h>

#include <cstddef>

struct memory_page
{
    static constexpr int size = 4096;

    template<typename T = void>
    static T* allocate(std::size_t count)
    {
         return (T*)mmap(NULL, memory_page::size * count,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANON,
                         -1, 0);
    }

    template<typename T>
    static int deallocate(T*          page,
                                 std::size_t count)
    {
        return munmap(page, memory_page::size * count);
    }
};

#endif
