#ifndef NEXIORA_RESEARCH_NX_AUTONOMOUS_RESEARCH_PIPELINE_H
#define NEXIORA_RESEARCH_NX_AUTONOMOUS_RESEARCH_PIPELINE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxArpStatus
{
    NX_ARP_STATUS_OK = 0,
    NX_ARP_STATUS_INVALID_ARGUMENT = 1,
    NX_ARP_STATUS_DISCOVERY_FAILED = 2,
    NX_ARP_STATUS_RUN_FAILED = 3,
    NX_ARP_STATUS_EVIDENCE_FAILED = 4,
    NX_ARP_STATUS_JOURNAL_FAILED = 5,
    NX_ARP_STATUS_MANIFEST_FAILED = 6,
    NX_ARP_STATUS_PROMOTION_EVALUATION_FAILED = 7
} NxArpStatus;

typedef enum NxArpRecommendation
{
    NX_ARP_RECOMMENDATION_NONE = 0,
    NX_ARP_RECOMMENDATION_REJECT = 1,
    NX_ARP_RECOMMENDATION_NEEDS_MORE_EVIDENCE = 2,
    NX_ARP_RECOMMENDATION_READY_FOR_HUMAN_REVIEW = 3
} NxArpRecommendation;

typedef struct NxAutonomousResearchPipelineCallbacks
{
    int (*discover_experiments)(void* user_data);
    int (*run_experiments)(void* user_data);
    int (*generate_evidence)(void* user_data);
    int (*write_journal)(void* user_data);
    int (*update_manifest)(void* user_data);
    int (*evaluate_promotion)(void* user_data, int* score_out);
} NxAutonomousResearchPipelineCallbacks;

typedef struct NxAutonomousResearchPipelineResult
{
    int promotion_score;
    NxArpRecommendation recommendation;
} NxAutonomousResearchPipelineResult;

NxArpStatus NxAutonomousResearchPipeline_Run(
    const NxAutonomousResearchPipelineCallbacks* callbacks,
    void* user_data,
    NxAutonomousResearchPipelineResult* result_out);

#ifdef __cplusplus
}
#endif

#endif
