#include "Nexiora/Research/NxResearchPolicy.h"

#include <stdio.h>

int main(void)
{
    NxResearchPolicy policy;
    NxResearchQueueItem item;

    NxResearchPolicy_Default(&policy);
    policy.minimum_priority = 50;
    policy.max_items_per_session = 2;

    item.id = 1;
    item.priority = 40;
    item.attempts = 1;
    item.max_attempts = 3;
    item.state = NX_RESEARCH_QUEUE_ITEM_RUNNING;

    if (NxResearchPolicy_Evaluate(&policy, &item, 0) != NX_RESEARCH_POLICY_DECISION_SKIP)
    {
        printf("expected low priority skip\n");
        return 1;
    }

    item.priority = 90;
    if (NxResearchPolicy_Evaluate(&policy, &item, 0) != NX_RESEARCH_POLICY_DECISION_RUN)
    {
        printf("expected run\n");
        return 1;
    }

    if (NxResearchPolicy_Evaluate(&policy, &item, 2) != NX_RESEARCH_POLICY_DECISION_STOP)
    {
        printf("expected stop after max items\n");
        return 1;
    }

    return 0;
}
