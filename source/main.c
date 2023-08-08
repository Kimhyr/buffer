#include "config.h"

#define PATH_PREFIX   "/home/k/projects/buffer/" 
#define TEST_FILE_1MB PATH_PREFIX "tests/file_1mb.txt"
#define TEST_FILE     PATH_PREFIX "source/buffer.c"

#include "log.h"
#include "buffer.h"

int main(int argc, const char** argv)
{
        (void)argc, (void)argv;
        struct buffer_flags flags = {.writable = 1};
        struct buffer buffer;
        if (initiate_buffer(&buffer, flags, TEST_FILE_1MB, sizeof(TEST_FILE_1MB)) == -1) {
                log_error("`initiate_buffer` failed.", 0);
                return 1;
        }

        destroy_buffer(&buffer);

        if (initiate_buffer(&buffer, flags, TEST_FILE, sizeof(TEST_FILE)) == -1) {
                log_error("`initiate_buffer` failed.", 0);
                return 1;
        }

        return 0;
}
