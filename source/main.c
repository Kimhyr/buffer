#include "config.h"

#define PATH_PREFIX   "/home/k/projects/buffer/" 

#define TEST_FILE PATH_PREFIX "large.txt"
#ifndef TEST_FILE
#       define TEST_FILE PATH_PREFIX "source/buffer.c"
#endif

#include <unistd.h>

#include "buffer.h"
#include "constants.h"
#include "log.h"
#include "system.h"

int main(int argc, const char** argv)
{
        (void)argc, (void)argv;

        /* Check the system and get variables. */

        /* Check to make sure the memory page size is 4,096 bytes. */
        long conf = sysconf(_SC_PAGESIZE);
        if (conf == -1) {
                log_error("Failed to `sysconf(_SC_PAGESIZE)`.", 0);
                return -1;
        } else if (conf != MEMORY_PAGE_SIZE) {
                log_error("Expected _SC_PAGESIZE to be %lu instead of %lu.",
                          MEMORY_PAGE_SIZE, conf);
                return -1;
        }

        /* Get the maximum amount of opened files. */
        conf = sysconf(_SC_OPEN_MAX);
        if (conf == -1) {
                log_error("Failed to `sysconf(_SC_OPEN_MAX)`.", 0);
                return -1;
        }
        system_information.opened_files_limit = conf;
        log_note("Opened files limit: %li.", conf);

        /* Load a file */
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
