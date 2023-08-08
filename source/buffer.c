#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"
#include "simd.h"
#include "source/memory.h"

#include "buffer.h"

int load_file_into_buffer(struct buffer*      buffer,
                          struct buffer_flags flags,
                          const char*         file_path,
                          size_t              file_path_length)
{
        /* Open the file. */
        buffer->file_handle = open(file_path, O_RDWR);
        if (buffer->file_handle == -1) {
                log_error("Failed to `open` the file.", 0);
                return -1;
        }
        log_note("File opened with handle %i.", (int)buffer->file_handle);

        /* Get statistics. */
        struct stat st;
        if (fstat(buffer->file_handle, &st) == -1) {
                log_error("Failed to `fstat` the file.", 0);
                return -1;
        }
        buffer->statistics.bytes         = st.st_size;
        buffer->statistics.full_segments = floor((float)buffer->statistics.bytes / BUFFER_SEGMENT_CAPACITY);
        buffer->statistics.pages         = ceil((float)buffer->statistics.bytes / MEMORY_PAGE_SIZE);
        log_note("Amounted %lu bytes, %lu full segments and %lu pages.",
                 buffer->statistics.bytes,
                 buffer->statistics.full_segments,
                 buffer->statistics.pages);

        /* Allocate memory for segment maps. */
        buffer->segment_map = (struct buffer_segment_map*)allocate_memory_page(buffer->statistics.pages);
        if (buffer->segment_map == MAP_FAILED) {
                log_error("Failed to allocated %lu memory pages for segment maps.",
                          buffer->statistics.pages);
                return -1;
        }
        log_note("Allocated %lu memory pages for segment maps.",
                 buffer->statistics.pages);

        /* Memory map the file. */
        buffer->segment_map->segment = (buffer_segment_t*)mmap(0, buffer->statistics.bytes,
                                                               PROT_READ | PROT_WRITE, MAP_PRIVATE,
                                                               buffer->file_handle, 0);
        if (buffer->segment_map->segment == MAP_FAILED) {
                log_error("Failed to memory map %lu bytes from file.", buffer->statistics.bytes);
                return -1;
        }
        log_note("Memory mapped %lu bytes from file.", buffer->statistics.bytes);

        struct buffer_segment_map* segment_map = buffer->segment_map;
        buffer_segment_t*          segment     = segment_map->segment;

        /* Map the file by page. */
        uint64_t count      = 0;
        uint64_t page_count = floor((float)buffer->statistics.bytes / MEMORY_PAGE_SIZE);
        for (;;) {
                for (int j = 0; j < BUFFER_PAGE_CAPACITY - 1; ++j) {
                        segment_map->next        = segment_map + 1;
                        segment_map->next->prior = segment_map;
                        segment_map->segment     = segment;
                        segment_map->mass        = BUFFER_SEGMENT_CAPACITY;
                        ++segment_map, ++segment;
                }

                segment_map->segment = segment;
                segment_map->mass    = BUFFER_SEGMENT_CAPACITY;

                if (++count == page_count) {
                        if ((buffer->statistics.bytes & (MEMORY_PAGE_SIZE - 1)) == 0) {
                                segment_map->next        = buffer->segment_map;
                                segment_map->next->prior = segment_map;
                                log_note("Solely mapped %lu pages.", count);
                                goto done_mapping;
                        }

                        segment_map->next        = segment_map + 1;
                        segment_map->next->prior = segment_map;
                        ++segment_map, ++segment;
                        break;
                }

                segment_map->next        = segment_map + 1;
                segment_map->next->prior = segment_map;
                ++segment_map, ++segment;
        }

        log_note("Mapped %lu pages.", count);

        /* Map the file by segment. */
        count = buffer->statistics.full_segments - page_count * 64;
        for (uint64_t i = 0; i < count; ++i) {
                segment_map->next        = segment_map + 1;
                segment_map->next->prior = segment_map;
                segment_map->segment     = segment;
                segment_map->mass        = BUFFER_SEGMENT_CAPACITY;
                ++segment_map, ++segment;
        }
        
        log_note("Mapped %lu segments.", count);

        /* Map the file by bytes. */
        count                    = buffer->statistics.bytes - count * 64 - page_count * 64 * 64;
        segment_map->next        = buffer->segment_map;
        segment_map->next->prior = segment_map;
        segment_map->segment     = segment;
        segment_map->mass        = count;
        log_note("Mapped %lu bytes.", count);

done_mapping:
        log_note("Mapped file.", 0);
    
        /* Set the remaining fields. */
        buffer->flags            = flags;
        buffer->file_path        = file_path;
        buffer->file_path_length = file_path_length;
        return 0;
}

int destroy_buffer(struct buffer* buffer)
{
        (void)buffer;
        return 0;
}

uint64_t fprint_buffer(struct buffer* buffer,
                       FILE*          file)
{
        struct buffer_segment_map* segment_map = buffer->segment_map;
        uint64_t count = 0;
        do {
                volatile char c = **segment_map->segment;
                uint64_t printed = fwrite(*segment_map->segment, 1, segment_map->mass, file);
                if (printed != segment_map->mass) {
                        log_error("Expected to print %lu instead of %lu.",
                                  segment_map->mass, printed);
                        return -1;
                }
                count += printed;
                segment_map = segment_map->next;
        } while (segment_map != buffer->segment_map);
        log_note("Printed %lu bytes.", count);
        return count;
}

uint64_t count_buffer_segments(struct buffer* buffer)
{
        uint64_t count = 0;
        struct buffer_segment_map* segment_map = buffer->segment_map;
        do {
                segment_map = segment_map->next;
                ++count;
        } while (segment_map != buffer->segment_map);
        return count;
}
