#include "Nexiora/Research/NxResearchPolicy.h"

void NxResearchPolicy_Default(NxResearchPolicy* policy)
{
    if (policy == 0)
    {
        return;
    }
    policy->minimum_priority = 0;
    policy->max_attempts_per_item = 3;
    policy->max_items_per_session = 64;
}

NxResearchPolicyDecision NxResearchPolicy_Evaluate(
    const NxResearchPolicy* policy,
    const NxResearchQueueItem* item,
    unsigned int executed_in_session)
{
    NxResearchPolicy fallback;
    const NxResearchPolicy* active_policy = policy;
    if (item == 0)
    {
        return NX_RESEARCH_POLICY_DECISION_STOP;
    }
    if (active_policy == 0)
    {
        NxResearchPolicy_Default(&fallback);
        active_policy = &fallback;
    }
    if (active_policy->max_items_per_session > 0 && executed_in_session >= active_policy->max_items_per_session)
    {
        return NX_RESEARCH_POLICY_DECISION_STOP;
    }
    if (item->priority < active_policy->minimum_priority)
    {
        return NX_RESEARCH_POLICY_DECISION_SKIP;
    }
    if (active_policy->max_attempts_per_item > 0 && item->attempts > active_policy->max_attempts_per_item)
    {
        return NX_RESEARCH_POLICY_DECISION_SKIP;
    }
    return NX_RESEARCH_POLICY_DECISION_RUN;
}
