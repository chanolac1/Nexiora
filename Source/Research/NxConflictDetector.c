#include "Nexiora/Research/NxConflictDetector.h"

NxConflictSeverity NxConflictDetector_Classify(const NxEvidenceCorrelationSummary* summary)
{
    int smaller_weight;
    int larger_weight;
    int ratio;

    if (summary == 0)
    {
        return NX_CONFLICT_SEVERITY_NONE;
    }

    if (summary->support_weight == 0 || summary->contradiction_weight == 0)
    {
        return NX_CONFLICT_SEVERITY_NONE;
    }

    smaller_weight = summary->support_weight < summary->contradiction_weight
        ? summary->support_weight
        : summary->contradiction_weight;
    larger_weight = summary->support_weight > summary->contradiction_weight
        ? summary->support_weight
        : summary->contradiction_weight;

    if (larger_weight == 0)
    {
        return NX_CONFLICT_SEVERITY_NONE;
    }

    ratio = (smaller_weight * 100) / larger_weight;

    if (ratio >= 75)
    {
        return NX_CONFLICT_SEVERITY_HIGH;
    }

    if (ratio >= 45)
    {
        return NX_CONFLICT_SEVERITY_MEDIUM;
    }

    return NX_CONFLICT_SEVERITY_LOW;
}

NxEvidenceCorrelationStatus NxConflictDetector_BuildReport(
    const NxEvidenceCorrelationSummary* summary,
    NxConflictReport* report_out)
{
    if (summary == 0 || report_out == 0)
    {
        return NX_EVIDENCE_CORRELATION_INVALID_ARGUMENT;
    }

    report_out->hypothesis_id = summary->hypothesis_id;
    report_out->support_weight = summary->support_weight;
    report_out->contradiction_weight = summary->contradiction_weight;
    report_out->balance = summary->support_weight - summary->contradiction_weight;
    report_out->severity = NxConflictDetector_Classify(summary);
    report_out->requires_human_review =
        report_out->severity == NX_CONFLICT_SEVERITY_MEDIUM ||
        report_out->severity == NX_CONFLICT_SEVERITY_HIGH;

    return NX_EVIDENCE_CORRELATION_OK;
}
