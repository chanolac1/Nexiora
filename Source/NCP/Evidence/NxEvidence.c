#include "Nexiora/NCP/Evidence/NxEvidence.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#if defined(_WIN32)
#include <direct.h>
#define nx_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#define nx_mkdir(path) mkdir(path, 0777)
#endif

static void nx_timestamp(char* buffer, size_t buffer_size) {
    time_t now = time(NULL);
    struct tm tm_value;
#if defined(_WIN32)
    localtime_s(&tm_value, &now);
#else
    localtime_r(&now, &tm_value);
#endif
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &tm_value);
}

static void nx_create_directory_if_missing(const char* path) {
    if (!path) {
        return;
    }
    if (nx_mkdir(path) != 0 && errno != EEXIST) {
        return;
    }
}

NxResult nx_evidence_ensure_directories(void) {
    nx_create_directory_if_missing("Benchmarks");
    nx_create_directory_if_missing("Benchmarks/History");
    nx_create_directory_if_missing("Benchmarks/Reports");
    nx_create_directory_if_missing("Benchmarks/Approvals");
    return NX_OK;
}

NxResult nx_evidence_read_latest_average(const char* history_path,
                                          const char* benchmark_name,
                                          double* average_nanoseconds,
                                          int* found) {
    if (!history_path || !benchmark_name || !average_nanoseconds || !found) {
        return NX_ERROR_ARGUMENT;
    }

    *found = 0;
    *average_nanoseconds = 0.0;

    FILE* file = fopen(history_path, "r");
    if (!file) {
        return NX_OK;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char name[192];
        unsigned long long iterations = 0;
        double total_seconds = 0.0;
        double average_ns = 0.0;
        char timestamp[64];

        int parsed = sscanf(line, "%63[^,],%191[^,],%llu,%lf,%lf",
                            timestamp, name, &iterations, &total_seconds, &average_ns);
        if (parsed == 5 && strcmp(name, benchmark_name) == 0) {
            (void)timestamp;
            (void)iterations;
            (void)total_seconds;
            *average_nanoseconds = average_ns;
            *found = 1;
        }
    }

    fclose(file);
    return NX_OK;
}

NxResult nx_evidence_append_history(const char* history_path,
                                    const NxBenchmarkResult* result) {
    if (!history_path || !result) {
        return NX_ERROR_ARGUMENT;
    }

    FILE* file = fopen(history_path, "a");
    if (!file) {
        return NX_ERROR_IO;
    }

    char timestamp[64];
    nx_timestamp(timestamp, sizeof(timestamp));
    fprintf(file, "%s,%s,%llu,%.9f,%.3f\n",
            timestamp,
            result->name ? result->name : "unknown",
            (unsigned long long)result->iterations,
            result->total_seconds,
            result->average_nanoseconds);
    fclose(file);
    return NX_OK;
}

NxEvidenceComparison nx_evidence_compare(double previous_average_nanoseconds,
                                         double current_average_nanoseconds,
                                         double tolerance_percent) {
    NxEvidenceComparison comparison;
    comparison.status = NX_EVIDENCE_NO_BASELINE;
    comparison.has_baseline = previous_average_nanoseconds > 0.0;
    comparison.previous_average_nanoseconds = previous_average_nanoseconds;
    comparison.current_average_nanoseconds = current_average_nanoseconds;
    comparison.percent_change = 0.0;
    comparison.requires_human_approval = 0;

    if (!comparison.has_baseline) {
        return comparison;
    }

    comparison.percent_change = ((previous_average_nanoseconds - current_average_nanoseconds) /
                                 previous_average_nanoseconds) * 100.0;

    if (comparison.percent_change > tolerance_percent) {
        comparison.status = NX_EVIDENCE_IMPROVED;
        comparison.requires_human_approval = 1;
    } else if (comparison.percent_change < -tolerance_percent) {
        comparison.status = NX_EVIDENCE_REGRESSION;
        comparison.requires_human_approval = 0;
    } else {
        comparison.status = NX_EVIDENCE_STABLE;
        comparison.requires_human_approval = 0;
    }

    return comparison;
}

NxResult nx_evidence_write_report(const char* report_path,
                                  const NxBenchmarkResult* result,
                                  const NxEvidenceComparison* comparison) {
    if (!report_path || !result || !comparison) {
        return NX_ERROR_ARGUMENT;
    }

    FILE* file = fopen(report_path, "w");
    if (!file) {
        return NX_ERROR_IO;
    }

    char timestamp[64];
    nx_timestamp(timestamp, sizeof(timestamp));
    fprintf(file, "Nexiora Evidence Report\n");
    fprintf(file, "Generated: %s\n", timestamp);
    fprintf(file, "Benchmark: %s\n", result->name ? result->name : "unknown");
    fprintf(file, "Iterations: %llu\n", (unsigned long long)result->iterations);
    fprintf(file, "TotalSeconds: %.9f\n", result->total_seconds);
    fprintf(file, "CurrentAverageNs: %.3f\n", result->average_nanoseconds);

    if (comparison->has_baseline) {
        fprintf(file, "PreviousAverageNs: %.3f\n", comparison->previous_average_nanoseconds);
        fprintf(file, "ChangePercent: %.3f\n", comparison->percent_change);
    } else {
        fprintf(file, "PreviousAverageNs: none\n");
        fprintf(file, "ChangePercent: none\n");
    }

    switch (comparison->status) {
        case NX_EVIDENCE_IMPROVED:
            fprintf(file, "Status: improved\n");
            fprintf(file, "HumanApprovalRequired: yes\n");
            break;
        case NX_EVIDENCE_REGRESSION:
            fprintf(file, "Status: regression\n");
            fprintf(file, "HumanApprovalRequired: no\n");
            break;
        case NX_EVIDENCE_STABLE:
            fprintf(file, "Status: stable\n");
            fprintf(file, "HumanApprovalRequired: no\n");
            break;
        case NX_EVIDENCE_NO_BASELINE:
        default:
            fprintf(file, "Status: no-baseline\n");
            fprintf(file, "HumanApprovalRequired: no\n");
            break;
    }

    fclose(file);
    return NX_OK;
}

void nx_evidence_print_comparison(const NxEvidenceComparison* comparison) {
    if (!comparison) {
        return;
    }

    printf("Evidence\n");
    if (!comparison->has_baseline) {
        printf("  Baseline: none\n");
        printf("  Status: first measurement recorded\n");
        return;
    }

    printf("  Previous: %.3f ns\n", comparison->previous_average_nanoseconds);
    printf("  Current: %.3f ns\n", comparison->current_average_nanoseconds);
    printf("  Change: %.3f %%\n", comparison->percent_change);

    switch (comparison->status) {
        case NX_EVIDENCE_IMPROVED:
            printf("  Status: improved candidate\n");
            printf("  Approval: human approval required before promotion\n");
            break;
        case NX_EVIDENCE_REGRESSION:
            printf("  Status: regression detected\n");
            break;
        case NX_EVIDENCE_STABLE:
            printf("  Status: stable within tolerance\n");
            break;
        case NX_EVIDENCE_NO_BASELINE:
        default:
            printf("  Status: no baseline\n");
            break;
    }
}
