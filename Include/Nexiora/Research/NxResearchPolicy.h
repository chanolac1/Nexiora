#ifndef NEXIORA_RESEARCH_NX_RESEARCH_POLICY_H
#define NEXIORA_RESEARCH_NX_RESEARCH_POLICY_H

#include "Nexiora/Research/NxResearchQueue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxResearchPolicyDecision
{
    NX_RESEARCH_POLICY_DECISION_RUN = 0,
    NX_RESEARCH_POLICY_DECISION_SKIP = 1,
    NX_RESEARCH_POLICY_DECISION_STOP = 2
} NxResearchPolicyDecision;

typedef struct NxResearchPolicy
{
    int minimum_priority;
    unsigned int max_attempts_per_item;
    unsigned int max_items_per_session;
} NxResearchPolicy;

void NxResearchPolicy_Default(NxResearchPolicy* policy);

NxResearchPolicyDecision NxResearchPolicy_Evaluate(
    const NxResearchPolicy* policy,
    const NxResearchQueueItem* item,
    unsigned int executed_in_session);

#ifdef __cplusplus
}
#endif

#endif
