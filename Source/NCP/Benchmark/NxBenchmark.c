#include "Nexiora/NCP/Benchmark/NxBenchmark.h"
#include <stdio.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

double nx_time_seconds(void) {
#if defined(_WIN32)
    static LARGE_INTEGER frequency;
    static int initialized = 0;
    LARGE_INTEGER counter;
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = 1;
    }
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
#endif
}

NxResult nx_benchmark_run(const char* name,
                          NxBenchmarkFunction function,
                          void* user_data,
                          uint64_t iterations,
                          NxBenchmarkResult* result) {
    if (!name || !function || iterations == 0 || !result) {
        return NX_ERROR_ARGUMENT;
    }

    double start = nx_time_seconds();
    for (uint64_t i = 0; i < iterations; ++i) {
        function(user_data);
    }
    double end = nx_time_seconds();

    result->name = name;
    result->iterations = iterations;
    result->total_seconds = end - start;
    result->average_nanoseconds = (result->total_seconds * 1000000000.0) / (double)iterations;
    return NX_OK;
}

void nx_benchmark_print(const NxBenchmarkResult* result) {
    if (!result) {
        return;
    }
    printf("Benchmark: %s\n", result->name);
    printf("  Iterations: %llu\n", (unsigned long long)result->iterations);
    printf("  Total: %.9f s\n", result->total_seconds);
    printf("  Average: %.3f ns\n", result->average_nanoseconds);
}
