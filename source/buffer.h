#ifndef BUFFER_H
#define BUFFER_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "memory.h"
#include "macros.h"

#define BUFFER_PAGE_CAPACITY    (64)
#define BUFFER_SEGMENT_CAPACITY (64)

/* A segment is a 64-byte wide line of arranged data.  Some benifets of a
   segment is the ability to use AVX-512 SIMD instructions on one and since
   64-bytes is the typical amount of bytes prefetch instructions prefetches,
   prefetching is more effective. */
typedef uint8_t          buffer_segment_t[BUFFER_SEGMENT_CAPACITY];

/* A segment map maps a segment with the mass (the amount of bytes contained)
   of the segment and the prior and next linked segments in the form of other
   segment maps.  In other words, a segment map is node of a linked list while
   a segment is the data of the linked list.  Since a segment map is 32 bytes
   wide, it's suggested to fill a memory page with 128 segment maps. */
struct buffer_segment_map
{
        struct buffer_segment_map* prior;
        struct buffer_segment_map* next;
        buffer_segment_t*          segment;
        uint64_t                   mass;
};

/* These flags determine the behavior of the buffer. */
struct buffer_flags
{
        uint64_t _readable : 1,    /* This flag's sole purpose is to imitate the
                                      `MAP_READ` `mmap` flag. */
                 writable  : 1,    /* Ability to write and erase bytes from the
                                      buffer. */
                 _file_loaded : 1; /* This is set when the buffer has a file
                                      loaded. */
};

/* Some statistics about the buffer. */
struct buffer_statistics
{
        /* The following fields take the semantic form of, "The total number of
           XXX." Where "XXX" is name of the variable. */
        uint64_t bytes;
        uint64_t full_segments;
        uint64_t pages;
};

/* TODO: Explain this. */
struct buffer
{
        struct buffer_segment_map* segment_map;
        struct buffer_flags        flags;
        struct buffer_statistics   statistics;

        /* File-related fields. */
        int64_t     file_handle;
        uint64_t    file_path_length;
        const char* file_path;
};

/* Loads the file at `file_path` into the buffer.
   Returns `-1` on failure;  otherwise, `0`. */
int load_file_into_buffer(struct buffer*      buffer,
                          struct buffer_flags flags,
                          const char*         file_path,
                          uint64_t            file_path_length);

/* Deallocates all the memory used by the buffer.
   Returns `-1` on failure;  otherwise, `0`. */
int destroy_buffer(struct buffer* buffer);

/* Prints the contents of the buffer in order.
   Returns `-1` on failure;  otherwise, the amount of bytes printed. */
uint64_t fprint_buffer(struct buffer* buffer,
                       FILE*          file);

uint64_t count_buffer_segments(struct buffer* buffer);

/* Traverses and mutates a buffer. */
struct buffer_cursor
{
        struct buffer*             buffer;
        struct buffer_segment_map* segment_map;
        uint64_t                   offset;
        uint8_t                    _padding[8];
};

/* Initializes the cursor with the given buffer.
   Returns `-1` on failure;  otherwise, `0`. */
int initialize_cursor(struct buffer_cursor* cursor, struct buffer* buffer);


#endif
