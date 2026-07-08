#include "Nexiora/Research/NxAutonomousResearchPipeline.h"

static NxArpRecommendation NxArpRecommendationFromScore(int score)
{
    if (score >= 90)
    {
        return NX_ARP_RECOMMENDATION_READY_FOR_HUMAN_REVIEW;
    }

    if (score >= 70)
    {
        return NX_ARP_RECOMMENDATION_NEEDS_MORE_EVIDENCE;
    }

    return NX_ARP_RECOMMENDATION_REJECT;
}

NxArpStatus NxAutonomousResearchPipeline_Run(
    const NxAutonomousResearchPipelineCallbacks* callbacks,
    void* user_data,
    NxAutonomousResearchPipelineResult* result_out)
{
    int score = 0;

    if (callbacks == 0 || result_out == 0)
    {
        return NX_ARP_STATUS_INVALID_ARGUMENT;
    }

    result_out->promotion_score = 0;
    result_out->recommendation = NX_ARP_RECOMMENDATION_NONE;

    if (callbacks->discover_experiments == 0 ||
        callbacks->run_experiments == 0 ||
        callbacks->generate_evidence == 0 ||
        callbacks->write_journal == 0 ||
        callbacks->update_manifest == 0 ||
        callbacks->evaluate_promotion == 0)
    {
        return NX_ARP_STATUS_INVALID_ARGUMENT;
    }

    if (callbacks->discover_experiments(user_data) != 0)
    {
        return NX_ARP_STATUS_DISCOVERY_FAILED;
    }

    if (callbacks->run_experiments(user_data) != 0)
    {
        return NX_ARP_STATUS_RUN_FAILED;
    }

    if (callbacks->generate_evidence(user_data) != 0)
    {
        return NX_ARP_STATUS_EVIDENCE_FAILED;
    }

    if (callbacks->write_journal(user_data) != 0)
    {
        return NX_ARP_STATUS_JOURNAL_FAILED;
    }

    if (callbacks->update_manifest(user_data) != 0)
    {
        return NX_ARP_STATUS_MANIFEST_FAILED;
    }

    if (callbacks->evaluate_promotion(user_data, &score) != 0)
    {
        return NX_ARP_STATUS_PROMOTION_EVALUATION_FAILED;
    }

    result_out->promotion_score = score;
    result_out->recommendation = NxArpRecommendationFromScore(score);

    return NX_ARP_STATUS_OK;
}
