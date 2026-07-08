#include "Nexiora/Research/NxConflictDetector.h"

#include <stdio.h>

int main(void)
{
    NxEvidenceCorrelationSummary summary;
    NxConflictReport report;
    NxEvidenceCorrelationStatus status;

    summary.hypothesis_id = 11u;
    summary.support_count = 2u;
    summary.contradiction_count = 2u;
    summary.neutral_count = 0u;
    summary.support_weight = 90;
    summary.contradiction_weight = 80;
    summary.neutral_weight = 0;
    summary.confidence_delta = 10;
    summary.verdict = NX_EVIDENCE_CORRELATION_VERDICT_CONFLICTING;

    if (NxConflictDetector_Classify(&summary) != NX_CONFLICT_SEVERITY_HIGH)
    {
        printf("Expected high conflict\n");
        return 1;
    }

    status = NxConflictDetector_BuildReport(&summary, &report);
    if (status != NX_EVIDENCE_CORRELATION_OK)
    {
        printf("Report failed\n");
        return 1;
    }

    if (report.hypothesis_id != 11u || report.balance != 10)
    {
        printf("Unexpected report values\n");
        return 1;
    }

    if (report.severity != NX_CONFLICT_SEVERITY_HIGH || report.requires_human_review == 0)
    {
        printf("Expected human review for high conflict\n");
        return 1;
    }

    summary.support_weight = 100;
    summary.contradiction_weight = 10;

    if (NxConflictDetector_Classify(&summary) != NX_CONFLICT_SEVERITY_LOW)
    {
        printf("Expected low conflict\n");
        return 1;
    }

    summary.contradiction_weight = 0;
    if (NxConflictDetector_Classify(&summary) != NX_CONFLICT_SEVERITY_NONE)
    {
        printf("Expected no conflict\n");
        return 1;
    }

    return 0;
}
