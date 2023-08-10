#ifndef BUFFER_H
#define BUFFER_H

#include <cstdint>
#include <cstdio>
#include <string_view>

#include "memory.h"

// A segment is a 64-byte wide line of arranged data.  Some benifets of a
// segment is the ability to use AVX-512 SIMD instructions on one and since
// 64-bytes is the typical amount of bytes prefetch instructions prefetches,
// prefetching is more effective.
struct buffer_segment
{
    static constexpr int capacity = 64;

    std::uint8_t bytes[capacity];
};

// 
struct buffer_segment_flags
{
    uint32_t 
             // Specifies that the segment is the first segment of the buffer.
             is_origin : 1,

             // This specifies that the segment is the first segment of the page.
             is_first : 1,

             // This specifies that the segment is the last segment of the page.
             is_last : 1,

             // If `is_page_head` is set, this specifies that the page is empty and
             // the next page is specified by the field `next` of
             // `struct buffer_map`.
             // NOTE: This is useful for recycling a buffer's memory.
             page_is_empty : 1;
};

// A segment map maps a segment with the mass (the amount of bytes contained)
// of the segment and the prior and next linked segments in the form of other
// segment maps.  In other words, a segment map is node of a linked list while
// a segment is the data of the linked list.  Since a segment map is 32 bytes
// wide, it's suggested to fill a memory page with 128 segment maps.
struct buffer_map
{
    buffer_map*          prior;
    buffer_map*          next;
    buffer_segment*      segment;
    uint32_t             mass;
    buffer_segment_flags flags;
};

// Flags that determine the behavior of the buffer.
struct [[gnu::packed]] buffer_flags
{
    uint8_t
        : 1, // The first is skipped to be compatable with open/fcntl and
             // protection flags.
        is_mutable      : 1 = false, // Specifies that the buffer can be
                                     // written to and erased from.
        is_full         : 1 = false,
        has_file_loaded : 1 = false; // This is set when the buffer has a file loaded.
};

// Some statistics about the buffer.
struct buffer_statistics
{
    uint64_t bytes;
    uint64_t segments;
    uint64_t pages;
};

// TODO: Explain this.
struct buffer
{
    // The mapping of the first segment of the buffer.
    buffer_map*       map;
    buffer_flags      flags;
    uint8_t           _padding[7];
    buffer_statistics statistics;

    // File-related fields.
    int64_t     file_handle;
    uint64_t    file_path_length;
    const char* file_path;

    // Deallocates all the memory used by the buffer.
    // Returns `false` on failure;  otherwise, `true`.
    // NOTE: This does not flush the buffer.
    bool destroy();

    // Loads the file at `file_path` with the load flags of `flags`.
    // Returns `false` on failure;  otherwise, `true`.
    bool load_file(buffer_flags     flags,
                   std::string_view file_path);

    // Prints the contents of `buffer` to `file`.
    // Returns `-1` on failure;  otherwise, the amount of bytes printed.
    std::uint64_t print(std::FILE* file = stdout);

    std::uint64_t count_segments();
};

// Traverses and mutates a buffer.
struct buffer_cursor
{
    buffer* buffer;

    // Positional fields.
    buffer_map* map;
    uint64_t    offset;

    uint8_t _padding[8];

    // Initializes `cursor` with `buffer`.
    // Returns `false` on failure;  otherwise, `true`.
    bool initialize(struct buffer& buffer);
};

static_assert(sizeof(buffer_segment) == 64, "");
static_assert(sizeof(buffer_map)     == 32, "");
static_assert(sizeof(buffer)         == 64, "");

#endif  // BUFFER_H
