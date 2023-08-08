#include "config.h"

#define PATH_PREFIX   "/home/k/projects/buffer/" 
#define TEST_FILE_1MB PATH_PREFIX "file_1mb.txt"
#define TEST_FILE     PATH_PREFIX "source/buffer.c"

#include "log.h"
#include "buffer.h"

int main(int argc, const char** argv)
{
        (void)argc, (void)argv;
        struct buffer_flags flags = {.writable = 1};
        struct buffer* buffer = create_buffer(
                flags,
                TEST_FILE_1MB,
                sizeof(TEST_FILE_1MB));
        if (buffer == NULL) {
                log_error("`create_buffer` failed.", 0);
                return 1;
        }
        destroy_buffer(buffer);

        return 0;
}
