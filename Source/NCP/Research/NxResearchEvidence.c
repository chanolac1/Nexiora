#include "Nexiora/NCP/Research/NxResearchEvidence.h"

#include <stdio.h>
#include <string.h>

static void nx_research_evidence_copy(char* dst, size_t dst_size, const char* src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";

    size_t i = 0;
    for (; i + 1 < dst_size && src[i] != '\0'; ++i)
        dst[i] = src[i];

    dst[i] = '\0';
}

const char* nx_research_evidence_status_to_string(NxResearchEvidenceStatus status)
{
    switch (status)
    {
        case NX_RESEARCH_EVIDENCE_VALID: return "Valid";
        case NX_RESEARCH_EVIDENCE_INCONCLUSIVE: return "Inconclusive";
        case NX_RESEARCH_EVIDENCE_REJECTED: return "Rejected";
        default: return "Unknown";
    }
}

NxResult nx_research_evidence_from_run(
    const NxExperimentRunResult* run,
    double baseline_seconds,
    NxResearchEvidence* out_evidence
)
{
    if (!run || !out_evidence)
        return NX_ERROR_ARGUMENT;

    memset(out_evidence, 0, sizeof(*out_evidence));

    nx_research_evidence_copy(
        out_evidence->experiment_id,
        sizeof(out_evidence->experiment_id),
        run->experiment_id
    );

    out_evidence->elapsed_seconds = run->elapsed_seconds;
    out_evidence->baseline_seconds = baseline_seconds;

    if (run->status != NX_EXPERIMENT_RUN_COMPLETED)
    {
        out_evidence->status = NX_RESEARCH_EVIDENCE_REJECTED;
        nx_research_evidence_copy(
            out_evidence->recommendation,
            sizeof(out_evidence->recommendation),
            "Experiment did not complete"
        );
        return NX_OK;
    }

    if (baseline_seconds <= 0.0)
    {
        out_evidence->status = NX_RESEARCH_EVIDENCE_INCONCLUSIVE;
        out_evidence->change_percent = 0.0;
        nx_research_evidence_copy(
            out_evidence->recommendation,
            sizeof(out_evidence->recommendation),
            "No baseline available; first evidence recorded"
        );
        return NX_OK;
    }

    out_evidence->change_percent =
        ((baseline_seconds - run->elapsed_seconds) / baseline_seconds) * 100.0;

    if (out_evidence->change_percent >= 1.0)
    {
        out_evidence->status = NX_RESEARCH_EVIDENCE_VALID;
        nx_research_evidence_copy(
            out_evidence->recommendation,
            sizeof(out_evidence->recommendation),
            "Candidate may proceed to review"
        );
    }
    else if (out_evidence->change_percent <= -1.0)
    {
        out_evidence->status = NX_RESEARCH_EVIDENCE_REJECTED;
        nx_research_evidence_copy(
            out_evidence->recommendation,
            sizeof(out_evidence->recommendation),
            "Regression detected"
        );
    }
    else
    {
        out_evidence->status = NX_RESEARCH_EVIDENCE_INCONCLUSIVE;
        nx_research_evidence_copy(
            out_evidence->recommendation,
            sizeof(out_evidence->recommendation),
            "Stable within tolerance"
        );
    }

    return NX_OK;
}

NxResult nx_research_evidence_write_report(
    const char* path,
    const NxResearchEvidence* evidence
)
{
    if (!path || !evidence)
        return NX_ERROR_ARGUMENT;

    FILE* file = fopen(path, "wb");
    if (!file)
        return NX_ERROR_IO;

    fprintf(file, "# Research Evidence Report\n\n");
    fprintf(file, "## Experiment\n%s\n\n", evidence->experiment_id);
    fprintf(file, "## Status\n%s\n\n", nx_research_evidence_status_to_string(evidence->status));
    fprintf(file, "## Elapsed Seconds\n%.9f\n\n", evidence->elapsed_seconds);
    fprintf(file, "## Baseline Seconds\n%.9f\n\n", evidence->baseline_seconds);
    fprintf(file, "## Change Percent\n%.3f\n\n", evidence->change_percent);
    fprintf(file, "## Recommendation\n%s\n", evidence->recommendation);

    fclose(file);
    return NX_OK;
}
