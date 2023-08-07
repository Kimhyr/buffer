#include <stdio.h>

#include "utils.h"
#include "buffer.h"

int main(int argc, char** argv)
{
        discard(argc, argv);

        const char file_path[] = "/home/k/projects/buffer/source/buffer.cc";
        struct buffer *b = create_buffer(
                buffer_flags {
                        .writable = true
                },
                file_path,
                sizeof(file_path)
        );

        if (b == nullptr) {
                fprintf(stderr, "Buffer failed to create");
        }

        return 0;
}
