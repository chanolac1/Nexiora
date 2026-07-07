#include "Nexiora/NCP/Benchmark/NxBenchmark.h"
#include "Nexiora/NCP/Evidence/NxEvidence.h"
#include "Nexiora/NCP/Memory/NxMemory.h"
#include "Nexiora/NCP/Hardware/NxHardware.h"
#include "Nexiora/NCP/String/NxString.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct NxArenaBenchmarkContext {
    NxArena arena;
} NxArenaBenchmarkContext;

static void benchmark_alloc_free(void* user_data) {
    (void)user_data;
    void* block = nx_memory_allocate(64);
    nx_memory_free(block);
}

static void benchmark_aligned_alloc_free(void* user_data) {
    (void)user_data;
    void* block = nx_memory_allocate_aligned(64, 64);
    nx_memory_free(block);
}

static void benchmark_arena_allocate_reset(void* user_data) {
    NxArenaBenchmarkContext* context = (NxArenaBenchmarkContext*)user_data;
    void* block = nx_arena_allocate(&context->arena, 64);
    (void)block;
    nx_arena_reset(&context->arena);
}


static void benchmark_hardware_query(void* user_data) {
    (void)user_data;
    NxHardwareInfo info;
    (void)nx_hardware_query(&info);
}


static void benchmark_string_length_64(void* user_data) {
    const char* text = (const char*)user_data;
    volatile size_t length = nx_string_length(text);
    (void)length;
}

static void benchmark_string_compare_64(void* user_data) {
    const char* text = (const char*)user_data;
    volatile int value = nx_string_compare(text, "Nexiora Core Platform string benchmark sample payload 0001");
    (void)value;
}

static void benchmark_string_copy_64(void* user_data) {
    const char* text = (const char*)user_data;
    char buffer[96];
    volatile NxResult result = nx_string_copy(buffer, sizeof(buffer), text);
    (void)result;
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

static int run_evidence_for_result(const NxBenchmarkResult* result,
                                   const char* history_path,
                                   const char* report_path) {
    const double tolerance_percent = 1.0;
    double previous_average_ns = 0.0;
    int found = 0;

    NxResult status = nx_evidence_read_latest_average(history_path, result->name, &previous_average_ns, &found);
    if (status != NX_OK) {
        printf("Evidence read failed: %d\n", (int)status);
        return 1;
    }

    NxEvidenceComparison comparison = found
        ? nx_evidence_compare(previous_average_ns, result->average_nanoseconds, tolerance_percent)
        : nx_evidence_compare(0.0, result->average_nanoseconds, tolerance_percent);

    nx_benchmark_print(result);
    nx_evidence_print_comparison(&comparison);

    status = nx_evidence_append_history(history_path, result);
    if (status != NX_OK) {
        printf("Evidence history write failed: %d\n", (int)status);
        return 1;
    }

    status = nx_evidence_write_report(report_path, result, &comparison);
    if (status != NX_OK) {
        printf("Evidence report write failed: %d\n", (int)status);
        return 1;
    }

    return comparison.status == NX_EVIDENCE_REGRESSION ? 2 : 0;
}

int main(int argc, char** argv) {
    const char* history_path = "Benchmarks/History/nexiora_bench_history.csv";
    const char* report_path = "Benchmarks/Reports/latest_evidence_report.txt";
    uint64_t iterations = parse_iterations(argc, argv);
    int final_status = 0;

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
    final_status |= run_evidence_for_result(&result, history_path, report_path);

    status = nx_benchmark_run("nx_memory_allocate_aligned/free 64 bytes",
                              benchmark_aligned_alloc_free,
                              NULL,
                              iterations,
                              &result);
    if (status != NX_OK) {
        printf("Benchmark failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }
    final_status |= run_evidence_for_result(&result, history_path, report_path);


    status = nx_benchmark_run("nx_hardware_query",
                              benchmark_hardware_query,
                              NULL,
                              iterations,
                              &result);
    if (status != NX_OK) {
        printf("Benchmark failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }
    final_status |= run_evidence_for_result(&result, history_path, report_path);

    NxArenaBenchmarkContext context;
    if (nx_arena_create(&context.arena, 4096, 64) != NX_OK) {
        printf("Arena benchmark setup failed.\n");
        nx_memory_shutdown();
        return 1;
    }

    status = nx_benchmark_run("nx_arena_allocate/reset 64 bytes",
                              benchmark_arena_allocate_reset,
                              &context,
                              iterations,
                              &result);
    nx_arena_destroy(&context.arena);
    if (status != NX_OK) {
        printf("Benchmark failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }
    final_status |= run_evidence_for_result(&result, history_path, report_path);


    const char* string_payload = "Nexiora Core Platform string benchmark sample payload 0001";

    status = nx_benchmark_run("nx_string_length 64 bytes",
                              benchmark_string_length_64,
                              (void*)string_payload,
                              iterations,
                              &result);
    if (status != NX_OK) {
        printf("Benchmark failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }
    final_status |= run_evidence_for_result(&result, history_path, report_path);

    status = nx_benchmark_run("nx_string_compare 64 bytes",
                              benchmark_string_compare_64,
                              (void*)string_payload,
                              iterations,
                              &result);
    if (status != NX_OK) {
        printf("Benchmark failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }
    final_status |= run_evidence_for_result(&result, history_path, report_path);

    status = nx_benchmark_run("nx_string_copy 64 bytes",
                              benchmark_string_copy_64,
                              (void*)string_payload,
                              iterations,
                              &result);
    if (status != NX_OK) {
        printf("Benchmark failed: %d\n", (int)status);
        nx_memory_shutdown();
        return 1;
    }
    final_status |= run_evidence_for_result(&result, history_path, report_path);

    nx_memory_print_stats();
    printf("Evidence report: %s\n", report_path);
    printf("Evidence history: %s\n", history_path);

    nx_memory_shutdown();
    return final_status == 0 ? 0 : final_status;
}
