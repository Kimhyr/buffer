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
        struct buffer_flags flags = {.writable = 1};
        struct buffer buffer;
        if (initiate_buffer(&buffer, flags, TEST_FILE, sizeof(TEST_FILE)) == -1) {
                log_error("`initiate_buffer` failed.", 0);
                return 1;
        }

        // log_note("buffer pages: %lu", count_buffer_pages(&buffer));

        // fwrite(buffer.page_map->prior->prior->prior->page->segments->bytes, 1, buffer.statistics.bytes, stdout);
        fprint_buffer(&buffer, stdout);
        

        return 0;
}
