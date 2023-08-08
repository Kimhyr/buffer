// FIXME: The buffer is not cyclic.

#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"
#include "simd.h"
#include "source/memory.h"

#include "buffer.h"

int initiate_buffer(struct buffer*      buffer,
                    struct buffer_flags flags,
                    const char*         file_path,
                    size_t              file_path_length)
{
        // Open the file.
        buffer->file_handle = open(file_path, O_RDWR);
        if (buffer->file_handle == -1) {
                log_error("Failed to `open` the file.", 0);
                return -1;
        }
        log_note("File opened with handle %i.", (int)buffer->file_handle);

        // Get statistics.
        struct stat st;
        if (fstat(buffer->file_handle, &st) == -1) {
                log_error("Failed to `fstat` the file.", 0);
                return -1;
        }
        buffer->statistics.bytes = st.st_size;
        if (buffer->statistics.bytes > BUFFER_CAPACITY) {
                log_error("The file's size (%lu) is to large.", buffer->statistics.bytes);
                return -1;
        }
        log_note("File has %lu bytes.", buffer->statistics.bytes);

        buffer->statistics.full_segments = floor((float)buffer->statistics.bytes / BUFFER_SEGMENT_SIZE);
        buffer->statistics.full_pages    = floor((float)buffer->statistics.full_segments / BUFFER_PAGE_SIZE);
        log_note("Amounted %lu full segments and %lu full pages.",
                 buffer->statistics.full_segments,
                 buffer->statistics.full_pages);

        // Allocate memory for the page maps.
        uint64_t page_map_count =
                ceil((float)buffer->statistics.bytes / (BUFFER_PAGE_MAP_CAPACITY * BUFFER_SEGMENT_SIZE));
        buffer->page_map = (struct buffer_page_map*)allocate_page(page_map_count);
        if (buffer->page_map == MAP_FAILED) {
                log_error("Failed to allocate %lu page maps.", page_map_count);
                return -1;
        }
        log_note("Allocated %lu page maps.", page_map_count);

        // Allocate memory for the file.
        buffer->page_map->page = (struct buffer_page*)
                mmap(0, buffer->statistics.bytes,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE, buffer->file_handle, 0);
        if (buffer->page_map->page == MAP_FAILED) {
                log_note("Failed to allocate %lu bytes for the file.", buffer->statistics.bytes);
                return -1;
        }
        log_note("Mapped %lu bytes from the file into memory.", buffer->statistics.bytes);

        // NOTE(Emhyr): Since `mmap` returns contiguous memory, we can use the
        // following variables as contiguous iterators.
        struct buffer_page_map*    page_map    = buffer->page_map;
        struct buffer_segment_map* segment_map = page_map->segment_maps;

        // Map the file by page.
        if (buffer->statistics.full_pages == 0) {
                goto skip_mapping_by_page;
        }
        
        for (uint64_t i = 0;;) {
                page_map->next = page_map + 1;
                page_map->page = buffer->page_map->page + i;

                for (uint64_t j = 0; j < BUFFER_PAGE_MAP_CAPACITY; ++j) {
                        segment_map->next        = segment_map + 1;

                        // FIXME: This accesses the next segment, but it
                        // doesn't signal a segfault when at the end of the
                        // page and the file can be mapped solely by pages.
                        segment_map->next->prior = segment_map;

                        segment_map->segment     = page_map->page->segments + j;
                        segment_map->size        = BUFFER_SEGMENT_SIZE;
                        ++segment_map;
                }

                if (++i == buffer->statistics.full_pages) {
                        if (buffer->statistics.full_pages < page_map_count) {
                                log_note("An additional page was created.", 0);
                                page_map->next->prior = page_map;
                                ++page_map;
                                page_map->page = buffer->page_map->page + i;
                        }
                        break;
                }

                page_map->next->prior = page_map;
                ++page_map;
        }

        uint64_t count;

        log_note("Mapped %lu pages.", buffer->statistics.full_pages);
        if (buffer->statistics.bytes % MEMORY_PAGE_SIZE == 0) {
                --segment_map;
                goto make_cyclic;
        }

skip_mapping_by_page:

        // Map the file by segment.
        count = buffer->statistics.full_segments
              - buffer->statistics.full_pages * BUFFER_PAGE_SIZE;
        for (uint64_t i = 0; i < count; ++i) {
                segment_map->next        = segment_map + 1;
                segment_map->next->prior = segment_map;
                segment_map->segment     = page_map->page->segments + i;
                segment_map->size        = BUFFER_SEGMENT_SIZE;
                ++segment_map;
        }
        log_note("Mapped %lu segments.", count);

        // Map the file by byte.
        count = buffer->statistics.bytes
              - count * BUFFER_SEGMENT_SIZE
              - buffer->statistics.full_pages * BUFFER_SEGMENT_SIZE * BUFFER_PAGE_SIZE;
        segment_map->segment     = page_map->page->segments + count;
        segment_map->size        = count;
        log_note("Mapped %lu bytes.", count);

make_cyclic:
        segment_map->next        = buffer->page_map->segment_maps;
        segment_map->next->prior = segment_map;

        log_note("Mapped file.", 0);

        // Make the pages cyclic.
        page_map->next        = buffer->page_map;
        page_map->next->prior = page_map;
    
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

        if (close(buffer->file_handle) == -1) {
                log_error("Failed to `munmap` page map.", 0);
                return -1;
        }
        log_note("File closed with handle %i.", (int)buffer->file_handle);

        return 0;
}

int fprint_buffer(struct buffer* buffer,
                  FILE*          file)
{
        if (buffer->statistics.bytes == 0) {
                log_warning("The buffer is empty.", 0);
                return 0;
        }

        uint64_t total_printed = 0;

        struct buffer_segment_map* segment_map = buffer->page_map->segment_maps;
        do {
                uint64_t printed =
                        fwrite(segment_map->segment->bytes,
                               1, segment_map->size, file);
                if (printed != segment_map->size) {
                        log_error("Expected %lu bytes instead of %lu.",
                                  segment_map->size, printed);
                        return -1;
                }
                total_printed += printed;
                segment_map = segment_map->next;
        } while (segment_map != buffer->page_map->segment_maps);

        if (fflush(file) == -1) {
                log_error("Failed to flush file %p.", file);
                return -1;
        }

        log_note("Printed %lu bytes.", total_printed);
        return 0;
}

void traverse_buffer(struct buffer* buffer)
{
        volatile uint64_t segment_count = 0;
        struct buffer_segment_map* segment_map = buffer->page_map->segment_maps;
        do {
                segment_count = segment_count + 1;
                volatile uint8_t c = segment_map->segment->bytes[0];
                (void)c;
                log_note("Traversed over segment %lu.", segment_count);
                segment_map = segment_map->next;
        } while (segment_map != buffer->page_map->segment_maps);
}

uint64_t count_buffer_pages(struct buffer* buffer)
{
        uint64_t count = 0;
        struct buffer_page_map* page = buffer->page_map;
        do {
                ++count;
                page = page->next;
        } while (page != buffer->page_map);
        return count;
}
