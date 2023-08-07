#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

#include "memory.h"

struct buffer_segment
{
        uint8_t bytes[64];
} __attribute__((aligned(MEMORY_PAGE_SIZE / 64)));

struct buffer_page
{
        struct buffer_segment segments[64];
} __attribute__((aligned(MEMORY_PAGE_SIZE)));

struct buffer_segment_map
{
        struct buffer_segment_map* prior;
        struct buffer_segment_map* next;
        struct buffer_segment*     segment;
        uint64_t            size;
} __attribute__((aligned(32)));

struct buffer_page_map
{
        struct buffer_segment_map segment_maps[64];
        struct buffer_page_map*   prior;
        struct buffer_page_map*   next;
        struct buffer_page*       page;
        uint8_t                   _padding[40];
} __attribute__((aligned(64)));

struct buffer_flags
{
        uint64_t _readable : 1,
                 writable  : 1;
};

struct buffer
{
                
        struct buffer_page_map* page_map;
        struct buffer_flags     flags;

        // Statistical fields.
        uint64_t byte_count;
        uint64_t segment_count;
        uint64_t pages_count;

        // File-related fields.
        int64_t  file_handle;
        uint64_t file_path_length;
        uint8_t  _padding[8];
        char     file_path[1];
};

struct buffer_cursor
{
        struct buffer*             buffer;
        struct buffer_segment_map* segment_map;
        uint64_t                   offset;
        uint8_t                    _padding[8];
};


#ifdef __cplusplus
extern "C" {
#endif

struct buffer* create_buffer(
        struct buffer_flags flags,
        const char*         file_path,
        uint64_t            file_path_length);

void destroy_buffer(struct buffer* buffer);

#ifdef __cplusplus
}
#endif

static_assert(sizeof(struct buffer_page) == MEMORY_PAGE_SIZE, "");
static_assert(sizeof(struct buffer_page_map) == sizeof(struct buffer_segment_map[64]) + 64, "");
static_assert(sizeof(struct buffer_flags) == 8, "");
static_assert(sizeof(struct buffer_cursor) == sizeof(uint64_t) * 4, "");

#endif
