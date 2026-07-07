#ifndef NEXIORA_NCP_EVIDENCE_NXEVIDENCE_H
#define NEXIORA_NCP_EVIDENCE_NXEVIDENCE_H

#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/NCP/Benchmark/NxBenchmark.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxEvidenceStatus {
    NX_EVIDENCE_NO_BASELINE = 0,
    NX_EVIDENCE_IMPROVED = 1,
    NX_EVIDENCE_REGRESSION = 2,
    NX_EVIDENCE_STABLE = 3
} NxEvidenceStatus;

typedef struct NxEvidenceComparison {
    NxEvidenceStatus status;
    int has_baseline;
    double previous_average_nanoseconds;
    double current_average_nanoseconds;
    double percent_change;
    int requires_human_approval;
} NxEvidenceComparison;

NxResult nx_evidence_ensure_directories(void);
NxResult nx_evidence_read_latest_average(const char* history_path,
                                          const char* benchmark_name,
                                          double* average_nanoseconds,
                                          int* found);
NxResult nx_evidence_append_history(const char* history_path,
                                    const NxBenchmarkResult* result);
NxEvidenceComparison nx_evidence_compare(double previous_average_nanoseconds,
                                         double current_average_nanoseconds,
                                         double tolerance_percent);
NxResult nx_evidence_write_report(const char* report_path,
                                  const NxBenchmarkResult* result,
                                  const NxEvidenceComparison* comparison);
void nx_evidence_print_comparison(const NxEvidenceComparison* comparison);

#ifdef __cplusplus
}
#endif

#endif
