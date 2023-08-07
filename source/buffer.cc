#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "buffer.h"
#include "simd.h"

[[gnu::always_inline]]
static inline void _map_by_segment(
        int                         index,
        struct buffer_segment_map **segment_map,
        struct buffer_page_map     *page_map)
{
        printf("info: mapping segment %i\n", index + 1);
        (*segment_map)->prior = &page_map->segment_maps[index - 1];
        (*segment_map)->next = &page_map->segment_maps[index + 1];
        (*segment_map)->segment = &page_map->page->segments[index];
        (*segment_map)->size = BUFFER_SEGMENT_CAPACITY;
        *segment_map = (*segment_map)->next;
}

struct buffer *create_buffer(
        struct buffer_flags  flags,
        const char          *file_path,
        size_t               file_path_length)
{
        // Allocate the buffer.
        auto *buffer = (struct buffer*)mmap(
                nullptr, PAGE_SIZE,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANON,
                -1, 0);

        printf("info: buffer allocated\n");

        // Load the file into memory.
        int fd = open(file_path, O_RDWR);
        struct stat st;
        if (fstat(fd, &st) != 0) {
                return nullptr;
        }
        int prot_flags = PROT_READ;
        prot_flags |= flags.writable;   
        auto* pages = (struct buffer_page*)mmap(
                buffer->page_map.page,
                st.st_size, prot_flags,
                MAP_PRIVATE, fd, 0);
        buffer->stats.bytes = st.st_size;
        buffer->stats.segments = ceil(buffer->stats.bytes / BUFFER_SEGMENT_CAPACITY);

        printf("info: file loaded of size %lu\n", buffer->stats.bytes);

        // Map the file.
        struct buffer_page_map *page_map = &buffer->page_map;
        struct buffer_segment_map *segment_map = &page_map->segment_maps[0];
        size_t i = 0;
        size_t bytes = buffer->stats.bytes;
        size_t count = 0;

        if (bytes < BUFFER_SEGMENT_CAPACITY) {
                goto map_file_by_byte;
        } else if (buffer->stats.segments < BUFFER_PAGE_CAPACITY) {
                goto map_file_by_segment;
        }

        count = floor(buffer->stats.segments / BUFFER_PAGE_CAPACITY);

        // Map the file by page.
        printf("info: mapping file by page\n");
        for (; i < count; ++i) {
                printf("info: mapping page %lu\n", i + 1);
                page_map->page = &pages[i];
                /// FIXME: #pragma GCC unroll(BUFFER_PAGE_CAPACITY - 1)
                for (int j = 0; j < BUFFER_PAGE_CAPACITY; ++j){
                        _map_by_segment(j, &segment_map, page_map);
                }

                page_map->next = (struct buffer_page_map*)mmap(
                        0, PAGE_SIZE,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANON,
                        -1,0);
                page_map = page_map->next;
                segment_map->next = &page_map->segment_maps[0];
        }

        if (buffer->stats.bytes % PAGE_SIZE == 0) {
                goto after_mapping;
        }

        bytes -= count * BUFFER_PAGE_CAPACITY * BUFFER_SEGMENT_CAPACITY;

        // Map the file by segment.
map_file_by_segment:
        printf("info: mapping file by segment\n");
        count = buffer->stats.segments - count * BUFFER_PAGE_CAPACITY;
        for (i = 0; i < count; ++i) {
                _map_by_segment(i, &segment_map, page_map);
        }

        bytes -= count * BUFFER_SEGMENT_CAPACITY;

        // Map the remaining bytes of the file.
map_file_by_byte:
        printf("info: mapping file by byte %lu\n", bytes);
        segment_map->segment = &page_map->page->segments[i];
        segment_map->size = bytes;

        // Turncating the rest of the page.
        printf("info: turncating page\n");
        count = &page_map->segment_maps[64] - &segment_map[1];
        (void)simd_memset(&segment_map[1], 0, count);

after_mapping:
        segment_map->next = &buffer->page_map.segment_maps[0];
        segment_map->next->prior = segment_map;

        // TODO: Initiate allocator

        // TODO: Initiate cursors

        // Set the remaining fields.
        printf("info: setting remaining fields\n");
        buffer->flags = flags;
        buffer->file_path_length = file_path_length;
        (void)simd_memcpy(
                (void*)&buffer->file_path[0],
                (void*)file_path, file_path_length
        );
        return buffer;
}

