#include "Nexiora/NCP/Benchmark/NxBenchmark.h"
#include "Nexiora/NCP/Evidence/NxEvidence.h"
#include "Nexiora/NCP/Memory/NxMemory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void benchmark_alloc_free(void* user_data) {
    (void)user_data;
    void* block = nx_memory_allocate(64);
    nx_memory_free(block);
}

static uint64_t parse_iterations(int argc, char** argv) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (strcmp(argv[i], "--iterations") == 0) {
            unsigned long long value = strtoull(argv[i + 1], NULL, 10);
            if (value > 0) {
                return (uint64_t)value;
            }
        }
    }
    return 100000;
}

int main(int argc, char** argv) {
    const char* history_path = "Benchmarks/History/nexiora_bench_history.csv";
    const char* report_path = "Benchmarks/Reports/latest_evidence_report.txt";
    const double tolerance_percent = 1.0;
    uint64_t iterations = parse_iterations(argc, argv);

    nx_evidence_ensure_directories();
    nx_memory_initialize();

    NxBenchmarkResult result;
    NxResult status = nx_benchmark_run("nx_memory_allocate/free 64 bytes",
                                       benchmark_alloc_free,
                                       NULL,
                                       iterations,
                                       &result);
    if (status != NX_OK) {
        printf("Benchmark failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }

    double previous_average_ns = 0.0;
    int found = 0;
    status = nx_evidence_read_latest_average(history_path, result.name, &previous_average_ns, &found);
    if (status != NX_OK) {
        printf("Evidence read failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }

    NxEvidenceComparison comparison = found
        ? nx_evidence_compare(previous_average_ns, result.average_nanoseconds, tolerance_percent)
        : nx_evidence_compare(0.0, result.average_nanoseconds, tolerance_percent);

    nx_benchmark_print(&result);
    nx_evidence_print_comparison(&comparison);

    status = nx_evidence_append_history(history_path, &result);
    if (status != NX_OK) {
        printf("Evidence history write failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }

    status = nx_evidence_write_report(report_path, &result, &comparison);
    if (status != NX_OK) {
        printf("Evidence report write failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }

    printf("Evidence report: %s\n", report_path);
    printf("Evidence history: %s\n", history_path);

    nx_memory_shutdown();
    return comparison.status == NX_EVIDENCE_REGRESSION ? 2 : 0;
}
