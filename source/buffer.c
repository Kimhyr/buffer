#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"
#include "simd.h"

#include "buffer.h"

int initiate_buffer(struct buffer*      buffer,
                    struct buffer_flags flags,
                    const char*         file_path,
                    size_t              file_path_length)
{
        log_note("Loading the file %s into the buffer...", file_path);
        buffer->file_handle = open(file_path, O_RDWR);
        if (buffer->file_handle == -1) {
                log_error("Failed to `open` the file.", 0);
                return -1;
        }

        // Get statistics.
        struct stat st;
        if (fstat(buffer->file_handle, &st) == -1) {
                log_error("Failed to `fstat` the file.", 0);
                return -1;
        }
        buffer->statistics.bytes = st.st_size;
        buffer->statistics.full_segments = floor(buffer->statistics.bytes / BUFFER_SEGMENT_SIZE);
        buffer->statistics.full_pages = floor(buffer->statistics.full_segments / BUFFER_PAGE_SIZE);
        log_note("bytes=%lu,full_segments=%lu,full_pages=%lu",
                 buffer->statistics.bytes, buffer->statistics.full_segments,
                 buffer->statistics.full_pages);

        // Allocate memory for the page maps.
        unsigned page_map_count = buffer->statistics.full_pages + 1;
        log_note("Allocating %u page maps...", page_map_count);
        struct buffer_page_map* page_maps = (struct buffer_page_map*)
                allocate_page(page_map_count);
        if (page_maps == MAP_FAILED) {
                log_error("Failed to allocate %u page maps.", page_map_count);
                return -1;
        }
        buffer->page_map = &page_maps[0];
        log_note("Allocated %u page maps.", page_map_count);

        // Allocate memory for the file.
        log_note("Allocating %lu bytes for the file...", buffer->statistics.bytes);
        struct buffer_page* pages = (struct buffer_page*)
                mmap(NULL, buffer->statistics.bytes,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE, buffer->file_handle, 0);
        if (pages == MAP_FAILED) {
                log_note("Failed to allocate %lu bytes for the file.", buffer->statistics.bytes);
                return -1;
        }
        buffer->page_map->page = &pages[0];
        log_note("Allocated %lu bytes for the file.", buffer->statistics.bytes);

        struct buffer_page_map* page_map = buffer->page_map;
        struct buffer_segment_map* segment_map = &page_map->segment_maps[0];

        // Map the file by page.
        log_note("Mapping %lu pages...", buffer->statistics.full_pages);
        for (unsigned i = 0; i < buffer->statistics.full_pages;) {
                segment_map->next = &page_map->segment_maps[1];
                segment_map->segment = &page_map->page->segments[0];
                segment_map->size = BUFFER_SEGMENT_SIZE;
                segment_map = segment_map->next;
                unsigned j = 1;
                do {
                        segment_map->prior = &page_map->segment_maps[j - 1];
                        segment_map->next = &page_map->segment_maps[j + 1];
                        segment_map->segment = &page_map->page->segments[j];
                        segment_map->size = BUFFER_SEGMENT_SIZE;
                        segment_map = segment_map->next;
                } while (++j < BUFFER_PAGE_MAP_SIZE - 1);
                segment_map->prior = &page_map->segment_maps[BUFFER_PAGE_MAP_SIZE - 1 - 1];
                segment_map->segment = &page_map->page->segments[BUFFER_PAGE_SIZE - 1];
                segment_map->size = BUFFER_SEGMENT_SIZE;
                page_map->next = &page_maps[++i + 1];
                page_map->next->prior = page_map;
                page_map = page_map->next;
                page_map->page = &pages[i];
                segment_map->next = &page_map->segment_maps[0];
                segment_map->next->prior = segment_map;
                segment_map = segment_map->next;
        }
        log_note("Mapped %lu pages.", buffer->statistics.full_pages);

        // Map the file by segment.
        unsigned full_segment_count = buffer->statistics.full_segments - buffer->statistics.full_pages * BUFFER_PAGE_SIZE;
        log_note("Mapping %lu segments...", full_segment_count);
        segment_map->next = &page_map->segment_maps[0];
        segment_map->segment = &page_map->page->segments[0];
        segment_map->size = BUFFER_PAGE_SIZE;
        segment_map->next->prior = segment_map;
        segment_map = segment_map->next;
        for (unsigned i = 1; i < full_segment_count; ++i) {
                segment_map->next = &page_map->segment_maps[i + 1];
                segment_map->segment = &page_map->page->segments[i];
                segment_map->size = BUFFER_PAGE_SIZE;
                segment_map->next->prior = segment_map;
                segment_map = segment_map->next;
        }
        log_note("Mapped %lu segments.", full_segment_count);

        // Map the file by byte.
        unsigned remaining_bytes = buffer->statistics.bytes - full_segment_count * BUFFER_SEGMENT_SIZE - buffer->statistics.full_pages * BUFFER_PAGE_SIZE * BUFFER_SEGMENT_SIZE;
        log_note("Mapping %lu bytes...", remaining_bytes);
        segment_map->next = &buffer->page_map->segment_maps[0];
        segment_map->next->prior = segment_map;
        segment_map->segment = &page_map->page->segments[full_segment_count];
        segment_map->size = remaining_bytes;
        log_note("Mapped %lu bytes.", remaining_bytes);

        // Make the buffer cyclic.
        page_map->next = &page_maps[page_map_count - 1];
        page_map->next->prior = page_map;
        segment_map->next = &buffer->page_map->segment_maps[0];
        segment_map->next->prior = segment_map;

        // Truncate the rest of the map.
        log_note("Truncating the pages...", 0);
        if (buffer->statistics.full_segments % BUFFER_PAGE_MAP_SIZE != 0) {
                char* start = (char*)&segment_map[1];
                simd_memset(start, 0, (char*)&page_map->page[1] - start);
        }
        simd_memset(&page_maps[page_map_count - 1], 0, MEMORY_PAGE_SIZE);
        log_note("Truncated the pages.", 0);

        log_note("Loaded the file %s into the buffer.", file_path);
     
        // Set the remaining fields.
        buffer->flags            = flags;
        buffer->file_path        = file_path;
        buffer->file_path_length = file_path_length;
        return 0;
}

int destroy_buffer(struct buffer* buffer)
{
        return 0;
        struct buffer_page_map* page_map = buffer->page_map;
        for (;;) {
                log_note("Memory unmapping page map...", 0);
                page_map = page_map->next;
                if (munmap(page_map->prior, MEMORY_PAGE_SIZE) == -1) {
                        log_error("Failed to `munmap` page map.", 0);
                        return -1;
                }

                log_note("Page map was memory unmapped.", 0);

                if (page_map == buffer->page_map) {
                        break;
                }
        }

        log_note("Closing the file...", 0);
        if (close(buffer->file_handle) == -1) {
                log_error("Failed to `munmap` page map.", 0);
                return -1;
        }

        return 0;
}
