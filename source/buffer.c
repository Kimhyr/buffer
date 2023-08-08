#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "log.h"
#include "simd.h"
#include "memory.h"
#include "error.h"

#include "buffer.h"

struct buffer* create_buffer(
        struct buffer_flags flags,
        const char*         file_path,
        size_t              file_path_length)
{
        log_note("Allocating the buffer...", 0);
        struct buffer* buffer = (struct buffer*)allocate_page(1);
        if (buffer == MAP_FAILED) {
                log_error("Failed to allocate the buffer.", 0);
                return NULL;
        }

        log_note("Loading the file into memory...", 0);
        buffer->file_handle = open(file_path, O_RDWR);
        if (buffer->file_handle == -1) {
                log_error("Failed to `open` the file.", 0);
                return NULL;
        }

        log_note("Getting the size of the file...", 0);
        struct stat st;
        if (fstat(buffer->file_handle, &st) == -1) {
                log_error("Failed to `fstat` the file.", 0);
                return NULL;
        }
        buffer->byte_count = st.st_size;
        log_note("Number of bytes: %i", buffer->byte_count);
        buffer->full_segment_count = floor(buffer->byte_count / BUFFER_SEGMENT_SIZE);
        log_note("Number of full segments: %i", buffer->full_segment_count);
        buffer->full_page_count = floor(buffer->full_segment_count / BUFFER_PAGE_SIZE);
        log_note("Number of full pages: %i", buffer->full_page_count);

        // NOTE: Allocate memory for the page maps.
        int page_map_count = buffer->full_page_count + 1;
        log_note("Allocating %i pages...", page_map_count);
        struct buffer_page_map* page_maps = (struct buffer_page_map*)
                allocate_page(page_map_count);
        if (page_maps == MAP_FAILED) {
                log_error("Failed to allocate pages for page maps.", 0);
                return NULL;
        }
        buffer->page_map = &page_maps[0];

        // NOTE: Allocate memory for the file.
        struct buffer_page* pages = (struct buffer_page*)mmap(
                NULL, buffer->byte_count,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE,
                buffer->file_handle, 0);
        if (pages == MAP_FAILED) {
                log_error("Failed to `mmap` the file into memory.", 0);
                return NULL;
        }
        buffer->page_map->page = &pages[0];
        
        log_note("Mapping the file...", 0);

        struct buffer_page_map* page_map = buffer->page_map;
        struct buffer_segment_map* segment_map = &page_map->segment_maps[0];

        // NOTE: Map the file by page.
        for (unsigned i = 0; i < buffer->full_page_count;) {
                log_note("Mapping page %u...", i);
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

        // NOTE: Map the file by segment.
        log_note("Mapping segment 0...", 0);
        unsigned full_segment_count = buffer->full_segment_count - buffer->full_page_count * BUFFER_PAGE_SIZE;
        segment_map->next = &page_map->segment_maps[0];
        segment_map->segment = &page_map->page->segments[0];
        segment_map->size = BUFFER_PAGE_SIZE;
        segment_map->next->prior = segment_map;
        segment_map = segment_map->next;
        for (unsigned i = 1; i < full_segment_count; ++i) {
                log_note("Mapping segment %u...", i);
                segment_map->next = &page_map->segment_maps[i + 1];
                segment_map->segment = &page_map->page->segments[i];
                segment_map->size = BUFFER_PAGE_SIZE;
                segment_map->next->prior = segment_map;
                segment_map = segment_map->next;
        }

        // NOTE: Map the file by byte.
        unsigned remaining = buffer->byte_count - full_segment_count * BUFFER_SEGMENT_SIZE - buffer->full_page_count * BUFFER_PAGE_SIZE * BUFFER_SEGMENT_SIZE;
        log_note("Mapping bytes %u...", remaining);
        segment_map->next = &buffer->page_map->segment_maps[0];
        segment_map->next->prior = segment_map;
        segment_map->segment = &page_map->page->segments[full_segment_count];
        segment_map->size = remaining;

        // NOTE: Make the buffer cyclic.
        log_note("Making the buffer cylclic...", 0);
        page_map->next = &page_maps[page_map_count - 1];
        page_map->next->prior = page_map;
        segment_map->next = &buffer->page_map->segment_maps[0];
        segment_map->next->prior = segment_map;

        // NOTE: Truncate the rest of the map.
        log_note("Truncating...", 0);
        if (buffer->full_segment_count % BUFFER_PAGE_MAP_SIZE != 0) {
                log_note("File size is not divisible by page size.", 0);
                char* start = (char*)&segment_map[1];
                simd_memset(start, 0, (char*)&page_map->page[1] - start);
        }
        simd_memset(&page_maps[page_map_count - 1], 0, MEMORY_PAGE_SIZE);

        // TODO: 
      
        // NOTE: Set the remaining fields.
        log_note("Setting the remaining fields...", 0);
        buffer->flags            = flags;
        buffer->file_path_length = file_path_length;
        (void)simd_memcpy(
                (void*)&buffer->file_path[0],
                (void*)file_path, file_path_length);
        return buffer;
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
                        merrno = 1;
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
                merrno = 2;
                return -1;
        }

        return 0;
}
