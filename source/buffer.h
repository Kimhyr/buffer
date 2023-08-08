#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stdio.h>

#include "memory.h"

#define BUFFER_CAPACITY          MIB * 4

#define BUFFER_SEGMENT_SIZE      64
#define BUFFER_PAGE_SIZE         64
#define BUFFER_PAGE_MAP_CAPACITY 64

struct buffer_segment
{
        uint8_t bytes[BUFFER_SEGMENT_SIZE];
};

struct buffer_page
{
        struct buffer_segment segments[BUFFER_PAGE_SIZE];
};

_Static_assert(sizeof(struct buffer_page) == MEMORY_PAGE_SIZE, "");

struct buffer_segment_map
{
        struct buffer_segment_map* prior;
        struct buffer_segment_map* next;
        struct buffer_segment*     segment;
        uint64_t                   size;
};

struct buffer_page_map
{
        struct buffer_segment_map segment_maps[BUFFER_PAGE_MAP_CAPACITY];
        struct buffer_page_map*   prior;
        struct buffer_page_map*   next;
        struct buffer_page*       page;
        uint8_t                   _padding[40];
} __attribute__((aligned(MEMORY_PAGE_SIZE)));

struct buffer_flags
{
        uint64_t _readable : 1,
                 writable  : 1;
};

struct buffer_statistics
{
        uint64_t bytes;
        uint64_t full_segments;
        uint64_t full_pages;
        uint8_t  _padding[8];
};

struct buffer
{
                
        struct buffer_page_map*  page_map;
        struct buffer_flags      flags;
        struct buffer_statistics statistics;

        // File-related fields.
        int64_t     file_handle;
        uint64_t    file_path_length;
        const char* file_path;
};

#ifdef __cplusplus
extern "C" {
#endif

// Returns `-1` on failure;  otherwise, `0`.
int initiate_buffer(struct buffer*      buffer,
                    struct buffer_flags flags,
                    const char*         file_path,
                    uint64_t            file_path_length);

// Returns `-1` on failure;  otherwise, `0`.
int destroy_buffer(struct buffer* buffer);

// Returns `-1` on failure;  otherwise, `0`.
int fprint_buffer(struct buffer* buffer,
                  FILE*          file);

void traverse_buffer(struct buffer* buffer);

uint64_t count_buffer_pages(struct buffer* buffer);

#ifdef __cplusplus
}
#endif

struct buffer_cursor
{
        struct buffer*             buffer;
        struct buffer_segment_map* segment_map;
        uint64_t                   offset;
        uint8_t                    _padding[8];
};

#endif
