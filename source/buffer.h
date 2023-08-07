#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdint.h>

#include "memory.h"

#define BUFFER_CAPACITY_SHIFT (6)
#define BUFFER_SEGMENT_CAPACITY (64)
#define BUFFER_PAGE_CAPACITY (64)

struct [[gnu::aligned(32)]]
        buffer_cursor
{
        struct buffer             *buffer;
        struct buffer_segment_map *segment_map;
        uint64_t                  offset;
        uint8_t                   _padding[8];
};

static_assert(sizeof(struct buffer_cursor) == sizeof(uint64_t) * 4);

struct [[gnu::aligned(32)]]
        buffer_cursor_list
{
        struct buffer_cursor_list *prior,
                                  *next;
        uint64_t                  cursor_count;
        uint8_t                   _padding[8];
};

static_assert(sizeof(struct buffer_cursor_list) == sizeof(struct buffer_cursor));

struct [[gnu::aligned(32)]]
        buffer_segment_map
{
        struct buffer_segment_map *prior,
                                  *next;
        struct buffer_segment     *segment;
        uint64_t                  size;
};

struct [[gnu::aligned(64)]]
        buffer_page_map
{
        struct buffer_segment_map segment_maps[64];
        struct buffer_page_map    *prior,
                                  *next;
        struct buffer_page        *page;
        uint8_t                   _padding[40];
};

static_assert(sizeof(struct buffer_page_map) == sizeof(struct buffer_segment_map[64]) + 64);

struct [[gnu::aligned(64)]]
        buffer_segment
{
        uint8_t bytes[BUFFER_SEGMENT_CAPACITY];
};

struct [[gnu::aligned(PAGE_SIZE)]]
        buffer_page
{
        buffer_segment segments[BUFFER_PAGE_CAPACITY];
};

static_assert(sizeof(struct buffer_page) == PAGE_SIZE);

struct buffer_flags
{
        uint64_t _readable : 1 = true,
                 writable  : 1;
};

struct buffer_allocator
{
        void *prior_position,
             *position;
};

struct buffer_stats
{
        uint64_t bytes;
        uint64_t segments;
};

struct buffer
{
        struct buffer_page_map page_map;
        struct buffer_cursor_list cursor_list;
        struct buffer_allocator allocator;
        struct buffer_stats stats;
        struct buffer_flags flags;
        uint64_t file_handle;
        uint64_t file_path_length;
        char file_path[1];
};

struct buffer *create_buffer(
        struct buffer_flags flags,
        const char         *file_path,
        size_t              file_path_length);

#endif
