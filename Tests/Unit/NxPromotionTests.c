#include "Nexiora/NCP/Research/NxPromotion.h"

#include <stdio.h>
#include <stdlib.h>

static int test_promotion_candidate_from_valid_evidence(void)
{
    NxResearchEvidence evidence;
    snprintf(evidence.experiment_id, sizeof(evidence.experiment_id), "LAB-0006");
    evidence.status = NX_RESEARCH_EVIDENCE_VALID;
    evidence.elapsed_seconds = 9.0;
    evidence.baseline_seconds = 10.0;
    evidence.change_percent = 10.0;
    snprintf(evidence.recommendation, sizeof(evidence.recommendation), "Candidate may proceed");

    NxPromotionReview review;
    if (nx_promotion_review_from_evidence(&evidence, &review) != NX_OK)
        return 1;

    if (review.decision != NX_PROMOTION_DECISION_CANDIDATE)
        return 2;

    if (!review.requires_human_approval)
        return 3;

    if (nx_promotion_write_report("Research/Reports/promotion_test.md", &review) != NX_OK)
        return 4;

    remove("Research/Reports/promotion_test.md");
    return 0;
}

static int test_promotion_rejects_regression(void)
{
    NxResearchEvidence evidence;
    snprintf(evidence.experiment_id, sizeof(evidence.experiment_id), "LAB-0006B");
    evidence.status = NX_RESEARCH_EVIDENCE_REJECTED;
    evidence.elapsed_seconds = 11.0;
    evidence.baseline_seconds = 10.0;
    evidence.change_percent = -10.0;
    snprintf(evidence.recommendation, sizeof(evidence.recommendation), "Regression detected");

    NxPromotionReview review;
    if (nx_promotion_review_from_evidence(&evidence, &review) != NX_OK)
        return 1;

    if (review.decision != NX_PROMOTION_DECISION_REJECTED)
        return 2;

    if (review.requires_human_approval)
        return 3;

    return 0;
}

int main(void)
{
    int rc = test_promotion_candidate_from_valid_evidence();
    if (rc != 0)
    {
        printf("NxPromotionTests candidate failed: %d\n", rc);
        return rc;
    }

    rc = test_promotion_rejects_regression();
    if (rc != 0)
    {
        printf("NxPromotionTests rejection failed: %d\n", rc);
        return rc;
    }

    printf("NxPromotionTests passed\n");
    return 0;
}
