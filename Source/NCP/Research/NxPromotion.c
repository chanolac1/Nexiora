#include "Nexiora/NCP/Research/NxPromotion.h"

#include <stdio.h>
#include <string.h>

static void nx_promotion_copy(char* dst, size_t dst_size, const char* src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";

    size_t i = 0;
    for (; i + 1 < dst_size && src[i] != '\0'; ++i)
        dst[i] = src[i];

    dst[i] = '\0';
}

const char* nx_promotion_decision_to_string(NxPromotionDecision decision)
{
    switch (decision)
    {
        case NX_PROMOTION_DECISION_CANDIDATE: return "Candidate";
        case NX_PROMOTION_DECISION_REJECTED: return "Rejected";
        case NX_PROMOTION_DECISION_NEEDS_MORE_DATA: return "NeedsMoreData";
        default: return "Unknown";
    }
}

NxResult nx_promotion_review_from_evidence(
    const NxResearchEvidence* evidence,
    NxPromotionReview* out_review
)
{
    if (!evidence || !out_review)
        return NX_ERROR_ARGUMENT;

    memset(out_review, 0, sizeof(*out_review));

    nx_promotion_copy(
        out_review->experiment_id,
        sizeof(out_review->experiment_id),
        evidence->experiment_id
    );

    out_review->requires_human_approval = 1;

    if (evidence->status == NX_RESEARCH_EVIDENCE_VALID)
    {
        out_review->decision = NX_PROMOTION_DECISION_CANDIDATE;
        out_review->confidence = 0.80;
        nx_promotion_copy(
            out_review->reason,
            sizeof(out_review->reason),
            "Evidence is valid; candidate requires human approval before promotion"
        );
        return NX_OK;
    }

    if (evidence->status == NX_RESEARCH_EVIDENCE_REJECTED)
    {
        out_review->decision = NX_PROMOTION_DECISION_REJECTED;
        out_review->confidence = 0.90;
        out_review->requires_human_approval = 0;
        nx_promotion_copy(
            out_review->reason,
            sizeof(out_review->reason),
            "Evidence rejects candidate or detects regression"
        );
        return NX_OK;
    }

    out_review->decision = NX_PROMOTION_DECISION_NEEDS_MORE_DATA;
    out_review->confidence = 0.50;
    nx_promotion_copy(
        out_review->reason,
        sizeof(out_review->reason),
        "Evidence is inconclusive; more benchmark data is required"
    );

    return NX_OK;
}

NxResult nx_promotion_write_report(
    const char* path,
    const NxPromotionReview* review
)
{
    if (!path || !review)
        return NX_ERROR_ARGUMENT;

    FILE* file = fopen(path, "wb");
    if (!file)
        return NX_ERROR_IO;

    fprintf(file, "# Promotion Review\n\n");
    fprintf(file, "## Experiment\n%s\n\n", review->experiment_id);
    fprintf(file, "## Decision\n%s\n\n", nx_promotion_decision_to_string(review->decision));
    fprintf(file, "## Confidence\n%.2f\n\n", review->confidence);
    fprintf(file, "## Human Approval Required\n%s\n\n", review->requires_human_approval ? "Yes" : "No");
    fprintf(file, "## Reason\n%s\n", review->reason);

    fclose(file);
    return NX_OK;
}
