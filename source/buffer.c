#include <math.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "buffer.h"
#include "simd.h"
#include "log.h"

struct buffer* create_buffer(
        struct buffer_flags flags,
        const char*         file_path,
        size_t              file_path_length)
{
        // Allocate the buffer.
        log_note("Allocating the buffer...", 0);
        struct buffer* buffer = (struct buffer*)mmap(
                NULL, MEMORY_PAGE_SIZE,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANON,
                -1, 0);
        assert(&buffer != MAP_FAILED);

        // Load the file into memory.
        log_note("Loading the file into memory...", 0);
        buffer->file_handle = open(file_path, O_RDWR);
        assert(buffer->file_handle != -1);
        struct stat st;
        assert(fstat(buffer->file_handle, &st) == 0);
        int32_t prot_flags = PROT_READ;
        prot_flags |= flags.writable; 

        // FIXME: `buffer->page_map->page` hasn't been initiated yet and it's
        // being used as a parameter here:  
        struct page* pages = (struct page*)mmap(
                buffer->page_map->page,
                st.st_size, prot_flags,
                MAP_PRIVATE, buffer->file_handle, 0);
        assert(pages != MAP_FAILED);
        buffer->byte_count = st.st_size;
        buffer->segment_count = ceil(buffer->byte_count / 64);

        // Map the file.
        log_note("Mapping the file...", 0);

        uint64_t count = 0;

        // NOTE: The file is being mapped by page.

        // NOTE: Get the amount of page maps needed.  Instead of calling
        // ceilling on the division, the division is floored because the first
        // page map has already been allocated.
        count = ceil(buffer->segment_count / 64);        
        log_note("Allocating %i pages...", count);
        buffer->page_map = (struct buffer_page_map*)mmap(
                0, MEMORY_PAGE_SIZE * count,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANON,
                -1, 0);
        while (--count) {
                // TODO: 
        }
        

        // TODO: The file is being mapped by segment.

        // TODO: The file is being mapped by byte.

        // TODO: The allocator is initiating.


        // Set the remaining fields.
        log_note("Setting the remaining fields...", 0);
        buffer->flags            = flags;
        buffer->file_path_length = file_path_length;
        (void)simd_memcpy(
                (void*)&buffer->file_path[0],
                (void*)file_path, file_path_length);
        return buffer;
}

void destroy_buffer(struct buffer* buffer)
{
        struct buffer_page_map* page_map = buffer->page_map;
        for (;;) {
                assert(munmap(page_map->page, MEMORY_PAGE_SIZE) == 0);
                if (page_map == buffer->page_map->prior) {
                        break;
                }
                page_map = page_map->next;
        }
        assert(close(buffer->file_handle) == 0);
}
