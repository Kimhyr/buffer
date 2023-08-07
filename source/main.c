#include "config.h"
 
#if !defined(BENCHMARK)

#include "buffer.h"

int main(int argc, const char** argv)
{
        (void)argc, (void)argv;
        const char file_path[] = "/home/k/projects/buffer/source/buffer.c";
        struct buffer_flags flags = {.writable = 1};
        struct buffer* buffer = create_buffer(
                flags,
                file_path,
                sizeof(file_path));
        destroy_buffer(buffer);
        return 0;
}

#endif

#if defined(BENCHMARK)

#if !defined(__cplusplus)
#       error "Compile as C++ to build benchmarks"
#endif

#include <benchmark/benchmark.h>
#include "buffer.h"

void do_benchmarks(benchmark::State& state)
{
        for (auto _ : state) {
                const char file_path[] = "/home/k/projects/buffer/source/buffer.cc";
                buffer(
                        { .writable = true },
                        file_path,
                        sizeof(file_path));
        }
}

BENCHMARK(do_benchmarks);
BENCHMARK_MAIN();

#endif
