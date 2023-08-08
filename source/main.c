#include "config.h"

#define PATH_PREFIX   "/home/k/projects/buffer/" 

#define TEST_FILE PATH_PREFIX "1mb.txt"
#ifndef TEST_FILE
#       define TEST_FILE PATH_PREFIX "source/buffer.c"
#endif

#include "log.h"
#include "buffer.h"

int main(int argc, const char** argv)
{
        (void)argc, (void)argv;
        struct buffer_flags flags = { .writable = 1 };
        struct buffer buffer;
        if (load_file_into_buffer(&buffer, flags, TEST_FILE, sizeof(TEST_FILE)) == -1) {
                log_error("`initiate_buffer` failed.", 0);
                return 1;
        }

        log_note("Buffer segment count: %lu ", count_buffer_segments(&buffer));
        log_note("Printed %lu bytes.", fprint_buffer(&buffer, stdout));
        return 0;
}
