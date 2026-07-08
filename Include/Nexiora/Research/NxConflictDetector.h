#ifndef NEXIORA_RESEARCH_NX_CONFLICT_DETECTOR_H
#define NEXIORA_RESEARCH_NX_CONFLICT_DETECTOR_H

#include "Nexiora/Research/NxEvidenceCorrelation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxConflictSeverity
{
    NX_CONFLICT_SEVERITY_NONE = 0,
    NX_CONFLICT_SEVERITY_LOW = 1,
    NX_CONFLICT_SEVERITY_MEDIUM = 2,
    NX_CONFLICT_SEVERITY_HIGH = 3
} NxConflictSeverity;

typedef struct NxConflictReport
{
    uint32_t hypothesis_id;
    int support_weight;
    int contradiction_weight;
    int balance;
    NxConflictSeverity severity;
    int requires_human_review;
} NxConflictReport;

NxConflictSeverity NxConflictDetector_Classify(const NxEvidenceCorrelationSummary* summary);

NxEvidenceCorrelationStatus NxConflictDetector_BuildReport(
    const NxEvidenceCorrelationSummary* summary,
    NxConflictReport* report_out);

#ifdef __cplusplus
}
#endif

#endif
