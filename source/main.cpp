#include "configuration.h"

#define PATH_PREFIX   "/home/k/projects/buffer/" 

#include <unistd.h>

#include <fstream>

#include "buffer.h"

int main(int argc, const char** argv)
{
    (void)argc, (void)argv;

    // Load a file
    const std::string_view file_path = PATH_PREFIX
#ifdef TEST_LARGE_FILE
        "large_file"
#elif defined(TEST_ALIGNED_FILE)
        "aligned_file"
#else
        "source/buffer.cpp"
#endif
    ;

    struct buffer_flags flags = { .is_mutable = 1 };
    struct buffer       buffer;
    if (!buffer.load_file(flags, file_path)) {
        return 1;
    }

    std::uint64_t segment_count;
    if ((segment_count = buffer.count_segments()) == (std::uint64_t)-1) {
        record_failure("Failed to count segments.");
        return 1;
    }
    record_note("Segment count: {}", segment_count);

    buffer.print();
    return 0;
}
