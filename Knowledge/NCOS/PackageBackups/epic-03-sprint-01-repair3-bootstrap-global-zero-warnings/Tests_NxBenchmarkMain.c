#include "Nexiora/NCP/Research/NxExperimentRunner.h"
#include "Nexiora/NCP/Research/NxJournal.h"
#include "Nexiora/NCP/Benchmark/NxBenchmark.h"
#include "Nexiora/NCP/Evidence/NxEvidence.h"
#include "Nexiora/NCP/Memory/NxMemory.h"
#include "Nexiora/NCP/Hardware/NxHardware.h"
#include "Nexiora/NCP/String/NxString.h"
#include "Nexiora/NCP/Containers/NxVector.h"
#include "Nexiora/Research/NxExperiment.h"
#include "Nexiora/Research/NxResearchKernel.h"
#include "Nexiora/Research/NxManifest.h"
#include "Nexiora/Research/NxRegistry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct NxBenchmarkTimer
{
    clock_t start;
} NxBenchmarkTimer;

static void nx_benchmark_timer_start(NxBenchmarkTimer* timer)
{
    timer->start = clock();
}

static double nx_benchmark_timer_stop(NxBenchmarkTimer* timer)
{
    return (double)(clock() - timer->start) / (double)CLOCKS_PER_SEC;
}

typedef enum NxBenchmarkModule {
    NX_BENCH_MODULE_ALL = 0,
    NX_BENCH_MODULE_MEMORY,
    NX_BENCH_MODULE_HARDWARE,
    NX_BENCH_MODULE_STRING,
    NX_BENCH_MODULE_CONTAINERS,
    NX_BENCH_MODULE_RESEARCH
} NxBenchmarkModule;

typedef struct NxArenaBenchmarkContext {
    NxArena arena;
} NxArenaBenchmarkContext;

typedef struct NxVectorBenchmarkContext {
    NxVector vector;
    int value;
} NxVectorBenchmarkContext;

typedef struct NxResearchBenchmarkContext {
    NxResearchKernel kernel;
    NxExperiment experiment;
    NxRegistry registry;
} NxResearchBenchmarkContext;

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

static void benchmark_vector_push_pop_i32(void* user_data) {
    NxVectorBenchmarkContext* context = (NxVectorBenchmarkContext*)user_data;
    (void)nx_vector_push_back(&context->vector, &context->value);
    int out = 0;
    (void)nx_vector_pop_back(&context->vector, &out);
}

static void benchmark_vector_at_i32(void* user_data) {
    NxVectorBenchmarkContext* context = (NxVectorBenchmarkContext*)user_data;
    volatile int* value = (volatile int*)nx_vector_at(&context->vector, 0);
    if (value != NULL) {
        volatile int copy = *value;
        (void)copy;
    }
}

static void benchmark_research_register_experiment(void* user_data) {
    NxResearchBenchmarkContext* context = (NxResearchBenchmarkContext*)user_data;
    context->kernel.experiment_count = 0;
    (void)nx_research_kernel_create_experiment(&context->kernel, &context->experiment);
}

static void benchmark_research_registry_add(void* user_data) {
    NxResearchBenchmarkContext* context = (NxResearchBenchmarkContext*)user_data;
    context->registry.count = 0;
    (void)nx_registry_add(&context->registry, &context->experiment);
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

static NxBenchmarkModule parse_module(int argc, char** argv) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (strcmp(argv[i], "--module") == 0) {
            const char* module = argv[i + 1];
            if (strcmp(module, "memory") == 0) {
                return NX_BENCH_MODULE_MEMORY;
            }
            if (strcmp(module, "hardware") == 0 || strcmp(module, "hal") == 0) {
                return NX_BENCH_MODULE_HARDWARE;
            }
            if (strcmp(module, "string") == 0 || strcmp(module, "strings") == 0) {
                return NX_BENCH_MODULE_STRING;
            }
            if (strcmp(module, "containers") == 0 || strcmp(module, "vector") == 0) {
                return NX_BENCH_MODULE_CONTAINERS;
            }
            if (strcmp(module, "research") == 0 || strcmp(module, "lab") == 0 || strcmp(module, "nrl") == 0) {
                return NX_BENCH_MODULE_RESEARCH;
            }
            if (strcmp(module, "all") == 0) {
                return NX_BENCH_MODULE_ALL;
            }
        }
    }
    return NX_BENCH_MODULE_ALL;
}

static int should_run(NxBenchmarkModule selected, NxBenchmarkModule candidate) {
    return selected == NX_BENCH_MODULE_ALL || selected == candidate;
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

static int run_one(const char* name,
                   NxBenchmarkFunction function,
                   void* user_data,
                   uint64_t iterations,
                   const char* history_path,
                   const char* report_path) {
    NxBenchmarkResult result;
    NxResult status = nx_benchmark_run(name, function, user_data, iterations, &result);
    if (status != NX_OK) {
        printf("Benchmark failed: %d\n", (int)status);
        return 1;
    }
    return run_evidence_for_result(&result, history_path, report_path);
}

static int run_memory_benchmarks(uint64_t iterations, const char* history_path, const char* report_path) {
    int final_status = 0;
    final_status |= run_one("nx_memory_allocate/free 64 bytes", benchmark_alloc_free, NULL, iterations, history_path, report_path);
    final_status |= run_one("nx_memory_allocate_aligned/free 64 bytes", benchmark_aligned_alloc_free, NULL, iterations, history_path, report_path);

    NxArenaBenchmarkContext context;
    if (nx_arena_create(&context.arena, 4096, 64) != NX_OK) {
        printf("Arena benchmark setup failed.\n");
        return 1;
    }
    final_status |= run_one("nx_arena_allocate/reset 64 bytes", benchmark_arena_allocate_reset, &context, iterations, history_path, report_path);
    nx_arena_destroy(&context.arena);

    return final_status;
}

static int run_hardware_benchmarks(uint64_t iterations, const char* history_path, const char* report_path) {
    return run_one("nx_hardware_query", benchmark_hardware_query, NULL, iterations, history_path, report_path);
}

static int run_string_benchmarks(uint64_t iterations, const char* history_path, const char* report_path) {
    int final_status = 0;
    const char* string_payload = "Nexiora Core Platform string benchmark sample payload 0001";
    final_status |= run_one("nx_string_length 64 bytes", benchmark_string_length_64, (void*)string_payload, iterations, history_path, report_path);
    final_status |= run_one("nx_string_compare 64 bytes", benchmark_string_compare_64, (void*)string_payload, iterations, history_path, report_path);
    final_status |= run_one("nx_string_copy 64 bytes", benchmark_string_copy_64, (void*)string_payload, iterations, history_path, report_path);
    return final_status;
}

static int run_container_benchmarks(uint64_t iterations, const char* history_path, const char* report_path) {
    int final_status = 0;
    NxVectorBenchmarkContext context;
    context.value = 42;

    if (nx_vector_create(&context.vector, sizeof(int), 1, NX_VECTOR_GROWTH_PERFORMANCE) != NX_OK) {
        printf("Vector benchmark setup failed.\n");
        return 1;
    }
    final_status |= run_one("nx_vector_push_pop_i32", benchmark_vector_push_pop_i32, &context, iterations, history_path, report_path);
    nx_vector_destroy(&context.vector);

    if (nx_vector_create(&context.vector, sizeof(int), 1, NX_VECTOR_GROWTH_PERFORMANCE) != NX_OK) {
        printf("Vector benchmark setup failed.\n");
        return 1;
    }
    if (nx_vector_push_back(&context.vector, &context.value) != NX_OK) {
        nx_vector_destroy(&context.vector);
        printf("Vector benchmark setup failed.\n");
        return 1;
    }
    final_status |= run_one("nx_vector_at_i32", benchmark_vector_at_i32, &context, iterations, history_path, report_path);
    nx_vector_destroy(&context.vector);

    return final_status;
}

static int run_research_benchmarks(uint64_t iterations, const char* history_path, const char* report_path) {
    int final_status = 0;
    NxResearchBenchmarkContext context;
    NxResearchKernelConfig config;
    config.root_path = "Research";
    config.registry_path = "Research/Registry/benchmark_registry.nxr";
    config.journal_path = "Research/Journals/benchmark_journal.log";

    if (nx_research_kernel_initialize(&context.kernel, &config) != NX_OK) {
        printf("Research benchmark setup failed.\n");
        return 1;
    }
    if (nx_experiment_initialize(&context.experiment,
                                 "LAB-BENCH-0001",
                                 "Research Register Benchmark",
                                 "Nexiora",
                                 "Research",
                                 "Measure the cost of registering one experiment in the NRL kernel.",
                                 "Research") != NX_OK) {
        nx_research_kernel_shutdown(&context.kernel);
        printf("Research benchmark setup failed.\n");
        return 1;
    }

    nx_registry_initialize(&context.registry);
    final_status |= run_one("nx_research_register_experiment", benchmark_research_register_experiment, &context, iterations, history_path, report_path);
    final_status |= run_one("nx_research_registry_add", benchmark_research_registry_add, &context, iterations, history_path, report_path);
    nx_research_kernel_shutdown(&context.kernel);
    return final_status;
}

static void print_usage(const char* executable) {
    printf("Usage: %s [--iterations N] [--module all|memory|hardware|string|containers|research]\n", executable);
}


static void nx_benchmark_research_journal_write(size_t iterations)
{
    const char* path = "Benchmarks/Reports/journal_benchmark.log";
    remove(path);

    NxJournal journal;
    if (nx_journal_open(&journal, path) != NX_OK)
    {
        printf("Benchmark: nx_research_journal_write\n  Failed to open journal\n");
        return;
    }

    NxBenchmarkTimer timer;
    nx_benchmark_timer_start(&timer);

    for (size_t i = 0; i < iterations; ++i)
    {
        NxJournalEvent event = nx_journal_make_event(
            NX_JOURNAL_EVENT_EXPERIMENT_STARTED,
            "LAB-0003",
            "JournalBenchmark",
            "benchmark event"
        );
        nx_journal_write_event(&journal, &event);
    }

    double total = nx_benchmark_timer_stop(&timer);
    double average_ns = (total * 1000000000.0) / (double)iterations;

    printf("Benchmark: nx_research_journal_write\n");
    printf("  Iterations: %zu\n", iterations);
    printf("  Total: %.9f s\n", total);
    printf("  Average: %.3f ns\n", average_ns);

    nx_journal_close(&journal);
}


static void nx_benchmark_research_runner_lifecycle(size_t iterations)
{
    const char* journal_path = "Benchmarks/Reports/runner_benchmark_journal.log";
    remove(journal_path);

    NxJournal journal;
    if (nx_journal_open(&journal, journal_path) != NX_OK)
    {
        printf("Benchmark: nx_research_runner_lifecycle\n  Failed to open journal\n");
        return;
    }

    NxExperimentRunner runner;
    nx_experiment_runner_initialize(&runner, "Research", &journal);

    NxBenchmarkTimer timer;
    nx_benchmark_timer_start(&timer);

    for (size_t i = 0; i < iterations; ++i)
    {
        NxExperimentRunResult result;
        nx_experiment_runner_execute(&runner, "LAB-0004", &result);
    }

    double total = nx_benchmark_timer_stop(&timer);
    double average_ns = (total * 1000000000.0) / (double)iterations;

    printf("Benchmark: nx_research_runner_lifecycle\n");
    printf("  Iterations: %zu\n", iterations);
    printf("  Total: %.9f s\n", total);
    printf("  Average: %.3f ns\n", average_ns);

    nx_experiment_runner_shutdown(&runner);
    nx_journal_close(&journal);
}

int main(int argc, char** argv) {
    const char* history_path = "Benchmarks/History/nexiora_bench_history.csv";
    const char* report_path = "Benchmarks/Reports/latest_evidence_report.txt";
    uint64_t iterations = parse_iterations(argc, argv);
    NxBenchmarkModule module = parse_module(argc, argv);
    int final_status = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    nx_evidence_ensure_directories();
    nx_memory_initialize();

    if (should_run(module, NX_BENCH_MODULE_MEMORY)) {
        final_status |= run_memory_benchmarks(iterations, history_path, report_path);
    }

    if (should_run(module, NX_BENCH_MODULE_HARDWARE)) {
        final_status |= run_hardware_benchmarks(iterations, history_path, report_path);
    }

    if (should_run(module, NX_BENCH_MODULE_STRING)) {
        final_status |= run_string_benchmarks(iterations, history_path, report_path);
    }

    if (should_run(module, NX_BENCH_MODULE_CONTAINERS)) {
        final_status |= run_container_benchmarks(iterations, history_path, report_path);
    }

    if (should_run(module, NX_BENCH_MODULE_RESEARCH)) {
        final_status |= run_research_benchmarks(iterations, history_path, report_path);
    }

    nx_memory_print_stats();
    printf("Evidence report: %s\n", report_path);
    printf("Evidence history: %s\n", history_path);

    nx_memory_shutdown();
    return final_status == 0 ? 0 : final_status;
}
