#ifndef NEXIORA_NCP_BENCHMARK_NXBENCHMARK_H
#define NEXIORA_NCP_BENCHMARK_NXBENCHMARK_H

#include <stdint.h>
#include "Nexiora/NCP/Common/NxResult.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*NxBenchmarkFunction)(void* user_data);

typedef struct NxBenchmarkResult {
    const char* name;
    uint64_t iterations;
    double total_seconds;
    double average_nanoseconds;
} NxBenchmarkResult;

NxResult nx_benchmark_run(const char* name,
                          NxBenchmarkFunction function,
                          void* user_data,
                          uint64_t iterations,
                          NxBenchmarkResult* result);

void nx_benchmark_print(const NxBenchmarkResult* result);

double nx_time_seconds(void);

#ifdef __cplusplus
}
#endif

#endif
