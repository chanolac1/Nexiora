#ifndef NEXIORA_NCP_RESEARCH_NXPROMOTION_H
#define NEXIORA_NCP_RESEARCH_NXPROMOTION_H

#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/NCP/Research/NxResearchEvidence.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NX_PROMOTION_EXPERIMENT_ID_MAX 32
#define NX_PROMOTION_REASON_MAX 256
#define NX_PROMOTION_REPORT_PATH_MAX 260

typedef enum NxPromotionDecision
{
    NX_PROMOTION_DECISION_UNKNOWN = 0,
    NX_PROMOTION_DECISION_CANDIDATE = 1,
    NX_PROMOTION_DECISION_REJECTED = 2,
    NX_PROMOTION_DECISION_NEEDS_MORE_DATA = 3
} NxPromotionDecision;

typedef struct NxPromotionReview
{
    char experiment_id[NX_PROMOTION_EXPERIMENT_ID_MAX];
    NxPromotionDecision decision;
    double confidence;
    char reason[NX_PROMOTION_REASON_MAX];
    int requires_human_approval;
} NxPromotionReview;

const char* nx_promotion_decision_to_string(NxPromotionDecision decision);

NxResult nx_promotion_review_from_evidence(
    const NxResearchEvidence* evidence,
    NxPromotionReview* out_review
);

NxResult nx_promotion_write_report(
    const char* path,
    const NxPromotionReview* review
);

#ifdef __cplusplus
}
#endif

#endif
