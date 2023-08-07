#if !defined(BENCHMARK)

#include "utils.h"

int main(int argc, const char **argv)
{
        discard(argc, argv);
        return 0;
}

#endif // !defined(_BENCHMARK)

#if defined(BENCHMARK)

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

#endif // defined(BENCHMARK)
