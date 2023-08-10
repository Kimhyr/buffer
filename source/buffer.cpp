#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cmath>

#include "debug.h"
#include "memory.h"
#include "simd.h"

#include "buffer.h"

bool buffer::load_file(buffer_flags     flags,
                       std::string_view file_path)
{
    // Open the file.
    if (this->flags.has_file_loaded) [[unlikely]] {
        record_failure("Buffer has file loaded");
        return false;
    }

    int open_flags    = 0 | (*(uint8_t*)&flags & O_RDWR);
    this->file_handle = open(file_path.begin(), open_flags);
    if (this->file_handle == -1) [[unlikely]] {
        record_failure("Failed to open the file: {}",
                       file_path);
        return false;
    }
    record_note("File opened with handle {}.", this->file_handle);

    // Get statistics.
    struct stat st;
    if (fstat(this->file_handle, &st) == -1) [[unlikely]] {
        record_failure("Failed to retrieve file statistics.");
        return false;
    }

    this->statistics.bytes      = st.st_size;
    float segments              = (float)this->statistics.bytes / buffer_segment::capacity;
    this->statistics.segments   = std::ceil(segments);
    std::uint64_t full_segments = std::floor(segments);
    this->statistics.pages      = std::ceil((float)this->statistics.segments / buffer_segment::capacity);
    record_note("The file can consume {} bytes, {} segments and {} pages.",
                this->statistics.bytes, this->statistics.segments,
                this->statistics.pages);

    long available_memory_pages = sysconf(_SC_AVPHYS_PAGES);
    if (available_memory_pages == -1) [[unlikely]] {
        record_failure("Failed to get the amount of available pages.");
        return false;
    }

    std::uint64_t minimum_available_pages = this->statistics.pages + 1;
    if (minimum_available_pages >= (std::uint64_t)available_memory_pages) [[unlikely]] {
        record_failure("The file can consume over the minimum available pages ({})",
                        minimum_available_pages);
        return false;
    }

    // Allocate memory for maps.
    this->map = memory_page::allocate<buffer_map>(this->statistics.pages);
    if (this->map == MAP_FAILED) [[unlikely]] {
        record_failure("Failed to allocate {} pages.",
                       this->statistics.pages);
        return false;
    }
    record_note("Allocated memory for maps.");

    // Memory map the file.
    int protection_flags = 1 | (1 >> this->flags.is_mutable);
    this->map->segment = (buffer_segment*)mmap(0, this->statistics.bytes,
                                                   protection_flags, MAP_PRIVATE,
                                                   this->file_handle, 0);
    if (this->map->segment == MAP_FAILED) [[unlikely]] {
    record_failure("Failed to memory map {} bytes from the file.",
                       this->statistics.bytes);
        return false;
    }
    record_note("Allocated memory for the file.");

    buffer_map* map         = this->map;
    buffer_segment* segment = map->segment;
    map->flags.is_origin    = true;

    // Map the file by page.
    std::uint64_t count      = 0;
    std::uint64_t page_count = std::floor((float)this->statistics.bytes / memory_page::size);
    for (;;) {
        map->flags.is_first = true;

        for (int j = 0; j < buffer_segment::capacity - 1; ++j) {
            map->next        = map + 1;
            map->next->prior = map;
            map->segment     = segment;
            map->mass        = buffer_segment::capacity;
            ++map, ++segment;
        }

        map->segment       = segment;
        map->mass          = buffer_segment::capacity;
        map->flags.is_last = true;

        if (++count == page_count) {
            if (is_aligned_by<memory_page::size>(this->statistics.bytes)) {
                record_note("The file's size is divisible by {}.",
                            memory_page::size);
                map->next        = this->map;
                map->next->prior = map;
                goto done_mapping;
            }

            map->next        = map + 1;
            map->next->prior = map;
            ++map, ++segment;
            break;
        }

        map->next        = map + 1;
        map->next->prior = map;
        ++map, ++segment;
    }

    // Map the file by segment.
    count = full_segments - page_count * 64;
    for (std::uint64_t i = 0; i < count; ++i) {
        map->next        = map + 1;
        map->next->prior = map;
        map->segment     = segment;
        map->mass        = buffer_segment::capacity;
        ++map, ++segment;
    }

    // Map the file by bytes.
    count            = this->statistics.bytes - count * 64 - page_count * 64 * 64;
    map->next        = this->map;
    map->next->prior = map;
    map->segment     = segment;
    map->mass        = count;

done_mapping:
    record_note("Successfully mapped the file.");

    // Set the remaining fields.
    this->flags                 = flags;
    this->flags.has_file_loaded = true;
    this->file_path             = file_path.begin();
    this->file_path_length      = file_path.length();
    return true;
}

bool buffer::destroy()
{
    return false;
}

std::uint64_t buffer::print(std::FILE* file)
{
    buffer_map* segment_map = this->map;
    std::uint64_t count       = 0;
    do {
        volatile char c = segment_map->segment->bytes[0];
        (void)c;
        std::uint64_t printed = std::fwrite(segment_map->segment->bytes,
                                            1, segment_map->mass, file);
        if (printed != segment_map->mass) {
            return -1;
        }
        count      += printed;
        segment_map = segment_map->next;
    } while (segment_map != this->map);
    return count;
}

std::uint64_t buffer::count_segments()
{
    std::uint64_t count     = 0;
    buffer_map* segment_map = this->map;
    do {
        segment_map = segment_map->next;
        ++count;
    } while (segment_map != this->map);
    return count;
}

bool buffer_cursor::initialize(struct buffer& buffer)
{
    if (buffer.flags.has_file_loaded == true) [[unlikely]] {
        return false;
    }

    this->buffer = &buffer;
    this->map    = buffer.map;
    this->offset = 0;
    return true;
}
